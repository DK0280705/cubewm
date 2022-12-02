#include "server.h"
#include "logger.h"
#include <ev++.h>
#include <stdexcept>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
extern "C" {
#define explicit _explicit
#include <xcb/bigreq.h>
#include <xcb/randr.h>
#include <xcb/shape.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <xcb/xkb.h>
#undef explicit
}

constexpr const char* wm_sn_name = "cubewm-WM-Sn";

static void assert_(bool b, const char* msg)
{
    if (!b) throw std::runtime_error(msg);
}

void Server::_get_timestamp()
{
    xcb_generic_event_t* event;
    xcb_window_t win = root_screen->root;
    xcb_atom_t atom = XCB_ATOM_SUPERSCRIPT_X;
    xcb_atom_t type = XCB_ATOM_CARDINAL;

    int flag[] = { XCB_EVENT_MASK_PROPERTY_CHANGE };
    xcb_change_window_attributes(conn, win, XCB_CW_EVENT_MASK, flag);
    xcb_change_property(conn, XCB_PROP_MODE_APPEND, win, atom, type, 32, 0, "");

    xcb_prefetch_maximum_request_length(conn);

    xcb_flush(conn);

    while ((event = xcb_wait_for_event(conn))) {
        if(XCB_EVENT_RESPONSE_TYPE(event) == XCB_PROPERTY_NOTIFY)
        {
            timestamp = ((xcb_property_notify_event_t*)event)->time;
            free(event);
            break;
        }
        free(event);
    }
}

Server::Server()
    : screen_id(0)
{
}

void Server::_get_wm_sn_atom()
{
    char *atom_name = xcb_atom_name_by_screen("WM", screen_id);
    assert_(atom_name, "Failed to get WM_Sn atom name");
    
    wm_sn_owner = xcb_generate_id(conn);
 
    xcb_intern_atom_reply_t* a_reply = xcb_intern_atom_reply(conn, xcb_intern_atom_unchecked(conn, 0, strlen(atom_name), atom_name), NULL);
    free(atom_name);
    assert_(a_reply, "Failed to intern WM_Sn atom");

    wm_sn = a_reply->atom;
    free(a_reply);
    
    xcb_get_selection_owner_reply_t* s_reply = xcb_get_selection_owner_reply(conn, xcb_get_selection_owner(conn, wm_sn), NULL);

    assert_((s_reply && s_reply->owner != XCB_NONE), "Another window manager is already running!");

    xcb_create_window(conn,
                      root_screen->root_depth,
                      wm_sn_owner,
                      root_screen->root,
                      -1, -1, 1, 1,
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      root_screen->root_visual,
                      0, NULL);
    xcb_change_property(conn,
                        XCB_PROP_MODE_REPLACE,
                        wm_sn_owner,
                        XCB_ATOM_WM_CLASS,
                        XCB_ATOM_STRING,
                        8,
                        (strlen(wm_sn_name) + 1) * 2,
                        fmt::format("{}\0{}\0", wm_sn_name, wm_sn_name).c_str());

    xcb_set_selection_owner(conn, wm_sn_owner, wm_sn, timestamp);

    unsigned int usleep_time = 100000; /* 0.1 seconds */
    int check_rounds = 150;            /* Wait for a maximum of 10 seconds */
    xcb_get_geometry_reply_t *g_reply = NULL;
    do {
        free(g_reply);
        usleep(usleep_time);
        assert_((check_rounds-- != 0),"The old window manager still playing");
        g_reply = xcb_get_geometry_reply(conn, xcb_get_geometry(conn, s_reply->owner), NULL);
    } while(g_reply);

    free(s_reply);
}

void Server::run()
{
    try {
        conn = xcb_connect(NULL, &screen_id);
        assert_(!xcb_connection_has_error(conn), "Couldn't open display!");
        
        // Get selection owner
        _get_wm_sn_atom(); 

        root_screen = xcb_aux_get_screen(conn, screen_id);
        assert_(root_screen, "Couldn't get root screen property!");

        xcb_prefetch_extension_data(conn, &xcb_xkb_id);
        xcb_prefetch_extension_data(conn, &xcb_shape_id);
        xcb_prefetch_extension_data(conn, &xcb_big_requests_id);
        xcb_prefetch_extension_data(conn, &xcb_randr_id);

        _get_timestamp();
    } catch (const std::exception& e) {
        Log::error(e.what());
        return;
    }

    Log::info("Server started");
    Log::info("Last timestamp: {}", timestamp);

    // Main loop
    xcb_generic_event_t* event;
    while ((event = xcb_wait_for_event(conn))) {
        switch (event->response_type){
        }
        free(event);
    }
    return;
}

Server::~Server()
{
    Log::info("Terminating");
    xcb_disconnect(conn);
}
