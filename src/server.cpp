#include "server.h"
#include "atoms.h"
#include "logger.h"
#include <stdexcept>

extern "C" {
#define explicit _explicit
#include <xcb/bigreq.h>
#include <xcb/randr.h>
#include <xcb/shape.h>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_event.h>
#include <xcb/xkb.h>
#include <xcb/xproto.h>
#undef explicit
}

constexpr const char* wm_sn_name = "cubewm-WM-Sn";

static void assert_(bool b, const char* msg)
{
    if (!b) throw std::runtime_error(msg);
}

void Server::_acquire_timestamp()
{
    xcb_generic_event_t* event;

    xcb_window_t win  = screen->root;
    xcb_atom_t   atom = XCB_ATOM_SUPERSCRIPT_X;
    xcb_atom_t   type = XCB_ATOM_CARDINAL;

    unsigned int flag[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_change_window_attributes(conn, win, XCB_CW_EVENT_MASK, flag);
    xcb_change_property(conn, XCB_PROP_MODE_APPEND, win, atom, type, 32, 0, "");

    xcb_prefetch_maximum_request_length(conn);

    xcb_flush(conn);

    while ((event = xcb_wait_for_event(conn))) {
        if (XCB_EVENT_RESPONSE_TYPE(event) == XCB_PROPERTY_NOTIFY) {
            timestamp = ((xcb_property_notify_event_t*)event)->time;
            free(event);
            break;
        }
        free(event);
    }
}

Server::Server() : cursor(*this) {}

void Server::_check_another_wm()
{
    const uint32_t    select_input_val = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;
    xcb_void_cookie_t cookie           = xcb_change_window_attributes_checked(conn, screen->root,
                                                                              XCB_CW_EVENT_MASK,
                                                                              &select_input_val);
    assert_(!xcb_request_check(conn, cookie),
            "Another window manager is already running! (X Server)");
}

void Server::_acquire_atoms()
{
#define xmacro(name)                                                                               \
    {                                                                                              \
        xcb_intern_atom_reply_t* reply =                                                           \
            xcb_intern_atom_reply(conn, xcb_intern_atom(conn, 0, strlen(#name), #name), NULL);     \
        assert_(reply, "Couldn't get atom name: " #name);                                          \
        name = reply->atom;                                                                        \
        free(reply);                                                                               \
    }

    ATOMS_XMACRO
#undef xmacro
}

void Server::_acquire_wm_sn()
{
    // For the time being, i'll use this "WM_S" instead of "WM"
    // As i use old i3 version instead of the newer ones.
    char* atom_name = xcb_atom_name_by_screen("WM_S", screen_id);
    assert_(atom_name, "Failed to get WM_Sn atom name");
    
    wm_sn_owner = xcb_generate_id(conn);

    xcb_intern_atom_reply_t* a_reply = xcb_intern_atom_reply(
        conn, xcb_intern_atom_unchecked(conn, false, strlen(atom_name), atom_name), NULL);
    free(atom_name);
    assert_(a_reply, "Failed to intern WM_Sn atom");

    wm_sn = a_reply->atom;
    free(a_reply);

    xcb_get_selection_owner_reply_t* s_reply =
        xcb_get_selection_owner_reply(conn, xcb_get_selection_owner(conn, wm_sn), NULL);
    assert_((!s_reply || s_reply->owner == XCB_NONE),
            "Another window manager is already running! (WM_Sn Selection)");

    xcb_create_window(conn, screen->root_depth, wm_sn_owner, screen->root, -1, -1, 1, 1, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, 0, NULL);
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, wm_sn_owner, XCB_ATOM_WM_CLASS,
                        XCB_ATOM_STRING, 8, (strlen(wm_sn_name) + 1) * 2,
                        fmt::format("{}\0{}\0", wm_sn_name, wm_sn_name).c_str());

    xcb_set_selection_owner(conn, wm_sn_owner, wm_sn, timestamp);
    free(s_reply);

    char                        buf[32] = {0};
    xcb_client_message_event_t* event   = (xcb_client_message_event_t*)buf;

    event->response_type  = XCB_CLIENT_MESSAGE;
    event->window         = screen->root;
    event->format         = 32;
    event->type           = MANAGER;
    event->data.data32[0] = timestamp;
    event->data.data32[1] = wm_sn;
    event->data.data32[2] = wm_sn_owner;

    xcb_send_event(conn, 0, screen->root, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (char*)event);
}

void Server::_load_cursors()
{
    assert_(xcb_cursor_context_new(conn, screen, &cursor._ctx) >= 0,
            "Cannot allocate xcursor context");
#define xmacro(constant, name) cursor[constant] = xcb_cursor_load_cursor(cursor._ctx, name)
    xmacro(XCURSOR_POINTER, "left_ptr");
    xmacro(XCURSOR_RESIZE_HORIZONTAL, "sb_h_double_arrow");
    xmacro(XCURSOR_RESIZE_VERTICAL, "sb_v_double_arrow");
    xmacro(XCURSOR_TOP_LEFT_CORNER, "top_left_corner");
    xmacro(XCURSOR_TOP_RIGHT_CORNER, "top_right_corner");
    xmacro(XCURSOR_BOTTOM_LEFT_CORNER, "bottom_left_corner");
    xmacro(XCURSOR_BOTTOM_RIGHT_CORNER, "bottom_left_corner");
    xmacro(XCURSOR_WATCH, "watch");
    xmacro(XCURSOR_MOVE, "fleur");
#undef xmacro
}

Server* Server::init()
{
     try {
        static Server srv;

        srv.conn = xcb_connect(NULL, &srv.screen_id);
        assert_(!xcb_connection_has_error(srv.conn), "Couldn't open display!");

        srv.screen = xcb_aux_get_screen(srv.conn, srv.screen_id);
        
        return &srv;
    } catch (const std::exception& e) {
        Log::error(e.what());
        return nullptr;
    }
}

void Server::run()
{
    xcb_prefetch_extension_data(conn, &xcb_xkb_id);
    xcb_prefetch_extension_data(conn, &xcb_shape_id);
    xcb_prefetch_extension_data(conn, &xcb_big_requests_id);
    xcb_prefetch_extension_data(conn, &xcb_randr_id);

    _acquire_timestamp();
    _acquire_atoms();
    _acquire_wm_sn();
    _check_another_wm();

    Log::info("Server started");
    Log::info("Last timestamp: {}", timestamp);

    _load_cursors();

    // Main loop
    xcb_generic_event_t* event;
    while ((event = xcb_wait_for_event(conn))) {
        switch (event->response_type) {
        }
        free(event);
    }
    return;
}

Server::Cursor::Cursor(const Server& srv) noexcept : _srv(srv) {}

void Server::Cursor::operator=(enum XCursor c)
{
    unsigned int flag[] = {_cursors[c]};
    xcb_change_window_attributes(_srv.conn, _srv.screen->root, XCB_CW_CURSOR, flag);
}

xcb_cursor_t& Server::Cursor::operator[](enum XCursor c)
{
    return _cursors[c];
}

Server::~Server()
{
    Log::info("Terminating");
    xcb_disconnect(conn);
}
