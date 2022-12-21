#include "server.h"
#include "error.h"
#include "logger.h"
#include "root.h"
#include "window.h"
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <stdlib.h>
#include <thread>

extern "C" {
#define explicit _explicit
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_event.h>
#include <xcb/xproto.h>
#undef explicit
}

Server::Server()
    : conn(xcb_connect(NULL, &default_screen))
    , timestamp(XCB_CURRENT_TIME)
{
    assert_runtime(!xcb_connection_has_error(conn), "Failed to initialize Server!");

    screen = xcb_aux_get_screen(conn, default_screen);
}

Server& Server::instance()
{
    static Server srv;
    return srv;
}

void Server::acquire_timestamp()
{
    uint32_t temp1[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
    uint32_t temp2[] = {0};

    // Initiate requests
    xcb_grab_server(conn);
    xcb_change_window_attributes(conn, root_window(), XCB_CW_EVENT_MASK, temp1);
    xcb_change_property(conn, XCB_PROP_MODE_APPEND, root_window(), XCB_ATOM_SUPERSCRIPT_X, XCB_ATOM_CARDINAL, 32, 0,
                        "");
    xcb_change_window_attributes(conn, root_window(), XCB_CW_EVENT_MASK, temp2);
    xcb_ungrab_server(conn);

    xcb_flush(conn);

    xcb_generic_event_t* event;
    while ((event = xcb_wait_for_event(conn))) {
        if (XCB_EVENT_RESPONSE_TYPE(event) == XCB_PROPERTY_NOTIFY) {
            timestamp = ((xcb_property_notify_event_t*)event)->time;
            free(event);
            return;
        } else free(event);
    }
}

void Server::run()
{
    Log::info("Server started");
    Log::info("Last timestamp: {}", timestamp);

    Log::debug("Initializing Root");
    root = &Root::instance(*this);

    xcb_flush(conn);

    // Handle some bogus race condition as explained in i3wm source code
    xcb_grab_server(conn);
    xcb_generic_event_t* event;
    xcb_aux_sync(conn);
    while ((event = xcb_poll_for_event(conn))) {
        if (event->response_type) {
            int type = event->response_type & 0x7F;
            if (type == XCB_MAP_REQUEST) handle(type, event);
        }
        free(event);
    }
    xcb_ungrab_server(conn);

    xcb_flush(conn);

    // main loop
    while ((event = xcb_wait_for_event(conn))) {
        int type = event->response_type & 0x7F;
        handle(type, event);
        free(event);
        xcb_flush(conn);
    }
}

void Server::on_key_press(const xcb_key_press_event_t& event) {}

void Server::on_button_press(const xcb_button_press_event_t& event) {}

void Server::on_motion_notify(const xcb_motion_notify_event_t& event) {}

void Server::on_enter_notify(const xcb_enter_notify_event_t& event)
{
    timestamp = event.time;
}

void Server::on_leave_notify(const xcb_leave_notify_event_t& event) {}

void Server::on_focus_in(const xcb_focus_in_event_t& event)
{
    if (event.event == root_window()) {}
}

void Server::on_focus_out(const xcb_focus_out_event_t& event) {}

void Server::on_expose(const xcb_expose_event_t& event) {}

void Server::on_destroy_notify(const xcb_destroy_notify_event_t& event) {}

void Server::on_unmap_notify(const xcb_unmap_notify_event_t& event) {}

void Server::on_map_request(const xcb_map_request_event_t& event)
{
    ignored_events.insert(event.sequence);

    // We're not mapping managed windows
    if (root->is_managed(event.window)) return;

    manage_window(event.window);
}

void Server::on_configure_notify(const xcb_configure_notify_event_t& event)
{
    if (event.event != root_window()) return;
    root->update_monitors();
}

void Server::on_configure_request(const xcb_configure_request_event_t& event) {}

void Server::on_property_notify(const xcb_property_notify_event_t& event) {}

void Server::on_selection_clear(const xcb_selection_clear_event_t& event)
{
    if (event.selection != WM_SN_ATOM) Log::error("SelectionClear for unknown selection");
    assert_runtime(false, "Lost WM_Sn Selection, exiting");
}

void Server::on_client_message(const xcb_client_message_event_t& event) {}

void Server::on_mapping_notify(const xcb_mapping_notify_event_t& event) {}

void Server::handle(int type, xcb_generic_event_t* event)
{
    Log::debug("event sequence: {}", event->sequence);
    if (ignored_events.contains(event->sequence)) return;

    switch (type) {
    case XCB_KEY_PRESS:
    case XCB_KEY_RELEASE:
        Log::debug("Key press event");
        on_key_press(*(xcb_key_press_event_t*)event);
        break;
    case XCB_BUTTON_PRESS:
    case XCB_BUTTON_RELEASE:
        on_button_press(*(xcb_button_press_event_t*)event);
        break;
    case XCB_MOTION_NOTIFY:
        on_motion_notify(*(xcb_motion_notify_event_t*)event);
        break;
    case XCB_ENTER_NOTIFY:
        on_enter_notify(*(xcb_enter_notify_event_t*)event);
        break;
    case XCB_LEAVE_NOTIFY:
        on_leave_notify(*(xcb_leave_notify_event_t*)event);
        break;
    case XCB_FOCUS_IN:
        on_focus_in(*(xcb_focus_in_event_t*)event);
        break;
    case XCB_FOCUS_OUT:
        on_focus_out(*(xcb_focus_out_event_t*)event);
        break;
    case XCB_EXPOSE:
        on_expose(*(xcb_expose_event_t*)event);
        break;
    case XCB_DESTROY_NOTIFY:
        on_destroy_notify(*(xcb_destroy_notify_event_t*)event);
        break;
    case XCB_UNMAP_NOTIFY:
        on_unmap_notify(*(xcb_unmap_notify_event_t*)event);
        break;
    case XCB_MAP_REQUEST:
        Log::debug("Map request event");
        on_map_request(*(xcb_map_request_event_t*)event);
        break;
    case XCB_CONFIGURE_NOTIFY:
        on_configure_notify(*(xcb_configure_notify_event_t*)event);
        break;
    case XCB_CONFIGURE_REQUEST:
        on_configure_request(*(xcb_configure_request_event_t*)event);
        break;
    case XCB_PROPERTY_NOTIFY:
        on_property_notify(*(xcb_property_notify_event_t*)event);
        break;
    case XCB_SELECTION_CLEAR:
        on_selection_clear(*(xcb_selection_clear_event_t*)event);
        break;
    case XCB_CLIENT_MESSAGE:
        on_client_message(*(xcb_client_message_event_t*)event);
        break;
    case XCB_MAPPING_NOTIFY:
        on_mapping_notify(*(xcb_mapping_notify_event_t*)event);
        break;
    }
}

xcb_atom_t Server::atom(const std::string& atom_name) const
{
    xcb_atom_t atom = XCB_NONE;

    // Though, i'm not sure this would fail. The pointer reply makes me a bit puzzled.
    auto* reply = xcb_intern_atom_reply(conn,
                                        xcb_intern_atom_unchecked(conn, false, atom_name.length(), atom_name.c_str()),
                                        0);

    if (reply) {
        atom = reply->atom;
        free(reply);
    }

    return atom;
}

std::string Server::atom_name(xcb_atom_t atom) const
{
    auto* reply = xcb_get_atom_name_reply(conn, xcb_get_atom_name(conn, atom), NULL);

    std::string name = xcb_get_atom_name_name(reply);
    free(reply);

    return name;
}

void Server::manage_window(xcb_window_t id) {}

Server::~Server()
{
    Log::info("Terminating");
    xcb_disconnect(conn);
}
