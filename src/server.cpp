#include "atoms.h"
#include "event.h"
#include "ewmh.h"
#include "server.h"
#include "root.h"
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
#include <xcb/xcb_keysyms.h>
#include <xcb/xkb.h>
#include <xcb/xproto.h>
#undef explicit
}

static void assert_(bool b, const char* msg)
{
    if (!b) throw std::runtime_error(msg);
}

Server::Server(xcb_connection_t* conn, int screen_id)
    : conn(conn)
    , screen_id(screen_id)
    , screen(xcb_aux_get_screen(conn, screen_id))
    , ewmh(EWMH::init(this))
    , cursor(*this)
{
}

Server* Server::init()
{
    int screen_id;
    xcb_connection_t* conn = xcb_connect(NULL, &screen_id);
    if (xcb_connection_has_error(conn)) return nullptr;

    static Server srv(conn, screen_id);
    return &srv;
}

void Server::_check_another_wm() const
{
    const uint32_t    select_input_val = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT;
    xcb_void_cookie_t cookie           = xcb_change_window_attributes_checked(conn, screen->root,
                                                                              XCB_CW_EVENT_MASK,
                                                                              &select_input_val);
    assert_(!xcb_request_check(conn, cookie),
            "Another window manager is already running! (X Server)");
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

void Server::_load_xkb()
{
    const xcb_query_extension_reply_t* reply = xcb_get_extension_data(conn, &xcb_xkb_id);
    xkb_support = reply->present;

    if (!xkb_support) {
        Log::error("xcb is not present on this server");
        return;
    }
    xcb_xkb_use_extension(conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
    xcb_xkb_select_events(conn, XCB_XKB_ID_USE_CORE_KBD,
                          XCB_XKB_EVENT_TYPE_STATE_NOTIFY | XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
                              XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY,
                          0,
                          XCB_XKB_EVENT_TYPE_STATE_NOTIFY | XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
                              XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY,
                          0xff, 0xff, NULL);
    const uint32_t mask = XCB_XKB_PER_CLIENT_FLAG_GRABS_USE_XKB_STATE |
                          XCB_XKB_PER_CLIENT_FLAG_LOOKUP_STATE_WHEN_GRABBED |
                          XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT;
    xcb_xkb_per_client_flags_reply_t* preply = xcb_xkb_per_client_flags_reply(
        conn, xcb_xkb_per_client_flags(conn, XCB_XKB_ID_USE_CORE_KBD, mask, mask, 0, 0, 0), NULL);

    if (!preply || !(preply->value & mask)) Log::error("Could not get xkb client flags");

    free(preply);
    xkb_base = reply->first_event;
}

void Server::_load_shape()
{
    const xcb_query_extension_reply_t* reply = xcb_get_extension_data(conn, &xcb_shape_id);
    if (!reply->present) {
        shape_support = 0;
        Log::error("shape is not present on this server");
        return;
    }
    shape_base = reply->first_event;
    xcb_shape_query_version_reply_t* version = xcb_shape_query_version_reply(conn, xcb_shape_query_version(conn), NULL);
    shape_support = version && version->minor_version >= 1;
    free(version);
}

void Server::_load_randr()
{
    const xcb_query_extension_reply_t* reply = xcb_get_extension_data(conn, &xcb_randr_id);
    randr_support = reply->present;
    if (!randr_support) {
        Log::error("randr is not present on this server");
        return;
    }
    randr_base = reply->first_event;
}

void Server::run()
{
    assert_(ewmh->acquire_timestamp(), "Bruh, how?");
    assert_(ewmh->acquire_selection_screen(true), "Failed to get selection screen");
    _check_another_wm();
    
    xcb_prefetch_extension_data(conn, &xcb_xkb_id);
    xcb_prefetch_extension_data(conn, &xcb_shape_id);
    xcb_prefetch_extension_data(conn, &xcb_randr_id);
    xcb_prefetch_extension_data(conn, &xcb_big_requests_id);
    
    _load_cursors();
    _load_xkb();
    _load_shape();
    _load_randr();

    root = Root::init(this);

    Log::info("Server started");
    Log::info("Last timestamp: {}", ewmh->timestamp);

    xcb_flush(conn);
    
    // Handle some bogus race condition
    xcb_grab_server(conn);
    xcb_generic_event_t* event;
    xcb_aux_sync(conn);
    while ((event = xcb_poll_for_event(conn))) {
        if (event->response_type) {
            int type = event->response_type & 0x7F;
            if (type == XCB_MAP_REQUEST) eh->handle(type, event);
        }
        free(event);
    }
    xcb_ungrab_server(conn);

    xcb_flush(conn);

    // main loop
    while ((event = xcb_wait_for_event(conn))) {
        int type = event->response_type & ~0x80;
        eh->handle(type, event);
        free(event);
        xcb_flush(conn);
    }
}

xcb_atom_t Server::atom(const char* atom_name)
{
    return xcb_intern_atom_reply(conn, xcb_intern_atom_unchecked(conn, false, strlen(atom_name), atom_name), 0)->atom; 
}

Server::Cursor::Cursor(const Server& srv) noexcept : _srv(srv) {}

void Server::Cursor::operator=(enum XCursor c) const
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
    xcb_cursor_context_free(cursor._ctx);
    xcb_disconnect(conn);
}
