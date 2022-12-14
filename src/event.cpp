#include "event.h"
#include "server.h"
#include <cstdint>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

void Event_handler::_key_press(const xcb_key_press_event_t& event)
{
}

void Event_handler::_button_press(const xcb_button_press_event_t& event)
{
}

void Event_handler::_motion_notify(const xcb_motion_notify_event_t& event)
{
}

void Event_handler::_enter_notify(const xcb_enter_notify_event_t& event)
{
}

void Event_handler::_leave_notify(const xcb_leave_notify_event_t& event)
{
}

void Event_handler::_focus_in(const xcb_focus_in_event_t& event)
{
}

void Event_handler::_focus_out(const xcb_focus_out_event_t& event)
{
}

void Event_handler::_expose(const xcb_expose_event_t& event)
{
}

void Event_handler::_destroy_notify(const xcb_destroy_notify_event_t& event)
{
}

void Event_handler::_unmap_notify(const xcb_unmap_notify_event_t& event)
{
}

void Event_handler::_map_request(const xcb_map_request_event_t& event)
{
    _ignored_events.insert(event.sequence);

    xcb_map_window(_srv->conn, event.window);
    if (_srv->get_win_by_window_id(event.window)) return;
    _srv->manage_window(event.window);
}

void Event_handler::_configure_notify(const xcb_configure_notify_event_t& event)
{
}

void Event_handler::_configure_request(const xcb_configure_request_event_t& event)
{
}

void Event_handler::_property_notify(const xcb_property_notify_event_t& event)
{
}

void Event_handler::_selection_clear(const xcb_selection_clear_event_t& event)
{
}

#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT 0
#define _NET_WM_MOVERESIZE_SIZE_TOP 1
#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT 2
#define _NET_WM_MOVERESIZE_SIZE_RIGHT 3
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT 4
#define _NET_WM_MOVERESIZE_SIZE_BOTTOM 5
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT 6
#define _NET_WM_MOVERESIZE_SIZE_LEFT 7
#define _NET_WM_MOVERESIZE_MOVE 8           /* movement only */
#define _NET_WM_MOVERESIZE_SIZE_KEYBOARD 9  /* size via keyboard */
#define _NET_WM_MOVERESIZE_MOVE_KEYBOARD 10 /* move via keyboard */
#define _NET_WM_MOVERESIZE_CANCEL 11        /* cancel operation */

#define _NET_MOVERESIZE_WINDOW_X (1 << 8)
#define _NET_MOVERESIZE_WINDOW_Y (1 << 9)
#define _NET_MOVERESIZE_WINDOW_WIDTH (1 << 10)
#define _NET_MOVERESIZE_WINDOW_HEIGHT (1 << 11)

/**
 * EWMH
 */
void Event_handler::_client_message(const xcb_client_message_event_t& event)
{
}

void Event_handler::_mapping_notify(const xcb_mapping_notify_event_t& event)
{
}

void Event_handler::handle(int type, xcb_generic_event_t* event)
{
    if (_ignored_events.contains(event->sequence)) return;

    switch (type) {
    case XCB_KEY_PRESS:
        _key_press(*(xcb_key_press_event_t*)event);
        break;
    case XCB_BUTTON_PRESS:
        _button_press(*(xcb_button_press_event_t*)event);
        break;
    case XCB_MOTION_NOTIFY:
        _motion_notify(*(xcb_motion_notify_event_t*)event);
        break;
    case XCB_ENTER_NOTIFY:
        _enter_notify(*(xcb_enter_notify_event_t*)event);
        break;
    case XCB_LEAVE_NOTIFY:
        _leave_notify(*(xcb_leave_notify_event_t*)event);
        break;
    case XCB_FOCUS_IN:
        _focus_in(*(xcb_focus_in_event_t*)event);
        break;
    case XCB_FOCUS_OUT:
        _focus_out(*(xcb_focus_out_event_t*)event);
        break;
    case XCB_EXPOSE:
        _expose(*(xcb_expose_event_t*)event);
        break;
    case XCB_DESTROY_NOTIFY:
        _destroy_notify(*(xcb_destroy_notify_event_t*)event);
        break;
    case XCB_UNMAP_NOTIFY:
        _unmap_notify(*(xcb_unmap_notify_event_t*)event);
        break;
    case XCB_MAP_REQUEST:
        _map_request(*(xcb_map_request_event_t*)event);
        break;
    case XCB_CONFIGURE_NOTIFY:
        _configure_notify(*(xcb_configure_notify_event_t*)event);
        break;
    case XCB_CONFIGURE_REQUEST:
        _configure_request(*(xcb_configure_request_event_t*)event);
        break;
    case XCB_PROPERTY_NOTIFY:
        _property_notify(*(xcb_property_notify_event_t*)event);
        break;
    case XCB_SELECTION_CLEAR:
        _selection_clear(*(xcb_selection_clear_event_t*)event);
        break;
    case XCB_CLIENT_MESSAGE:
        _client_message(*(xcb_client_message_event_t*)event);
        break;
    case XCB_MAPPING_NOTIFY:
        _mapping_notify(*(xcb_mapping_notify_event_t*)event);
        break;
    }
}
