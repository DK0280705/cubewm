#include "config.h"
#include "keybind.h"
#include "server.h"
#include "atoms.h"
#include "monitormanager.h"
#include "windowmanager.h"
#include "workspacemanager.h"
#include "container.h"
#include "window.h"
#include "logger.h"
#include "xwrap.h"
#include <csignal>

#define explicit _explicit
#include <xcb/xkb.h>
#include <xcb/xproto.h>
#include <xcb/xcb.h>
#undef explicit

void Server::on_key_press(const xcb_key_press_event_t& event)
{
    if ((event.state & XCB_MOD_MASK_4) == XCB_MOD_MASK_4) {
        Log::debug("Pressed mod"); 
    } 
}

void Server::on_button_press(const xcb_button_press_event_t& event) {}

void Server::on_motion_notify(const xcb_motion_notify_event_t& event) {}

void Server::on_enter_notify(const xcb_enter_notify_event_t& event)
{
    _conn.timestamp.update(&event.time);
}

void Server::on_leave_notify(const xcb_leave_notify_event_t& event)
{
}

void Server::on_focus_in(const xcb_focus_in_event_t& event)
{
    if (event.event == root_window()) {}
}

void Server::on_focus_out(const xcb_focus_out_event_t& event) {}

void Server::on_expose(const xcb_expose_event_t& event) {}

void Server::on_destroy_notify(const xcb_destroy_notify_event_t& event)
{
    const xcb_unmap_notify_event_t unmap {
        .sequence = event.sequence,
        .event    = event.event,
        .window   = event.window
    };
    on_unmap_notify(unmap);
}

void Server::on_unmap_notify(const xcb_unmap_notify_event_t& event)
{
    if (!_window_manager.is_managed(event.window)) {
        Log::info("Ignoring UnmapNotify event (window is not managed)");
        return;
    }
    Window* win = _window_manager.at(event.window);
    Window_container* con = win->container;
    _workspace_manager.purge_container(con);
    _window_manager.unmanage(event.window);
    delete con;
}

void Server::on_map_request(const xcb_map_request_event_t& event)
{
    _ignored_events.insert(event.sequence); 

    if (!window_manageable(_window_manager, event.window, false)) {
        Log::info("Can't manage window");
        return;
    }

    Window* win = _window_manager.manage(event.window);

    Window_container* con = new Window_container(win);
    _workspace_manager.place_container(_workspace_manager.current(), con);
}

void Server::on_configure_notify(const xcb_configure_notify_event_t& event)
{
    if (event.event != root_window()) return;
    _monitor_manager.update();
}

void Server::on_configure_request(const xcb_configure_request_event_t& event) {}

void Server::on_property_notify(const xcb_property_notify_event_t& event) {}

void Server::on_selection_clear(const xcb_selection_clear_event_t& event)
{
    if (event.selection != Atom::WM_SN) Log::error("SelectionClear for unknown selection");
    Log::info("Lost selection because SelectionClear event");
    std::raise(SIGTERM);
}

void Server::on_client_message(const xcb_client_message_event_t& event) {}

void Server::on_mapping_notify(const xcb_mapping_notify_event_t& event) {}

void Server::handle(const xcb_generic_event_t& event)
{
    if (_ignored_events.contains(event.sequence)) return;

    const int type = event.response_type & ~0x80;

    if (Config::xkb_base != 0 && type == Config::xkb_base) {
        _keybind.handle(event);
        return;
    }

    switch (type) {
    case XCB_KEY_PRESS:
        Log::debug("Key press event");
        on_key_press(*(xcb_key_press_event_t*)(&event));
        break;
    case XCB_BUTTON_PRESS:
        Log::debug("Button press event");
        on_button_press(*(xcb_button_press_event_t*)(&event));
        break;
    case XCB_MOTION_NOTIFY:
        // Log::debug("Motion notify event"); // mega spam
        on_motion_notify(*(xcb_motion_notify_event_t*)(&event));
        break;
    case XCB_ENTER_NOTIFY:
        Log::debug("Enter notify event");
        on_enter_notify(*(xcb_enter_notify_event_t*)(&event));
        break;
    case XCB_LEAVE_NOTIFY:
        Log::debug("Leave notify event");
        on_leave_notify(*(xcb_leave_notify_event_t*)(&event));
        break;
    case XCB_FOCUS_IN:
        Log::debug("Focus in event");
        on_focus_in(*(xcb_focus_in_event_t*)(&event));
        break;
    case XCB_FOCUS_OUT:
        Log::debug("Focus out event");
        on_focus_out(*(xcb_focus_out_event_t*)(&event));
        break;
    case XCB_EXPOSE:
        Log::debug("Expose event");
        on_expose(*(xcb_expose_event_t*)(&event));
        break;
    case XCB_DESTROY_NOTIFY:
        Log::debug("Destroy notify event");
        on_destroy_notify(*(xcb_destroy_notify_event_t*)(&event));
        break;
    case XCB_UNMAP_NOTIFY:
        Log::debug("Unmap notify event");
        on_unmap_notify(*(xcb_unmap_notify_event_t*)(&event));
        break;
    case XCB_MAP_REQUEST:
        Log::debug("Map request event");
        on_map_request(*(xcb_map_request_event_t*)(&event));
        break;
    case XCB_CONFIGURE_NOTIFY:
        Log::debug("Configure notify event");
        on_configure_notify(*(xcb_configure_notify_event_t*)(&event));
        break;
    case XCB_CONFIGURE_REQUEST:
        Log::debug("Configure request event");
        on_configure_request(*(xcb_configure_request_event_t*)(&event));
        break;
    case XCB_PROPERTY_NOTIFY:
        Log::debug("Property notify event");
        on_property_notify(*(xcb_property_notify_event_t*)(&event));
        break;
    case XCB_SELECTION_CLEAR:
        Log::debug("Selection clear event");
        on_selection_clear(*(xcb_selection_clear_event_t*)(&event));
        break;
    case XCB_CLIENT_MESSAGE:
        Log::debug("Client message event");
        on_client_message(*(xcb_client_message_event_t*)(&event));
        break;
    case XCB_MAPPING_NOTIFY:
        Log::debug("Mapping notify event");
        on_mapping_notify(*(xcb_mapping_notify_event_t*)(&event));
        break;
    }
}

