#pragma once
#include "connection.h"
#include "rect.h"
#include <span>
#include <string>
#include <unordered_set>

extern "C" {
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xproto.h>
}

class Server
{
public: 
    static Server& init(Connection& conn);

    void start();
    void restart();
    void stop();

    inline xcb_window_t root_window() const
    { return _conn.screen()->root; }

    inline Rectangle default_rect() const
    {
        return {0, 0, _conn.screen()->width_in_pixels,
                _conn.screen()->height_in_pixels};
    }
    
    void handle(int type, xcb_generic_event_t* event);

    // Handler functions
    void on_key_press(const xcb_key_press_event_t& event);
    void on_button_press(const xcb_button_press_event_t& event);
    void on_motion_notify(const xcb_motion_notify_event_t& event);
    void on_enter_notify(const xcb_enter_notify_event_t& event);
    void on_leave_notify(const xcb_leave_notify_event_t& event);
    void on_focus_in(const xcb_focus_in_event_t& event);
    void on_focus_out(const xcb_focus_out_event_t& event);
    void on_expose(const xcb_expose_event_t& event);
    void on_destroy_notify(const xcb_destroy_notify_event_t& event);
    void on_unmap_notify(const xcb_unmap_notify_event_t& event);
    void on_map_request(const xcb_map_request_event_t& event);
    void on_configure_notify(const xcb_configure_notify_event_t& event);
    void on_configure_request(const xcb_configure_request_event_t& event);
    void on_property_notify(const xcb_property_notify_event_t& event);
    void on_selection_clear(const xcb_selection_clear_event_t& event);
    void on_client_message(const xcb_client_message_event_t& event);
    void on_mapping_notify(const xcb_mapping_notify_event_t& event);

    Server(const Server&)             = delete;
    Server(const Server&&)            = delete;
    Server& operator=(const Server&)  = delete;
    Server& operator=(const Server&&) = delete;
    ~Server();

private:
    Connection& _conn;

    bool _terminating;
    
    class Window_manager&    _window_manager;    // #include "windowmanager.h"
    class Monitor_manager&   _monitor_manager;   // #include "monitormanager.h"
    class Workspace_manager& _workspace_manager; // #include "workspacemaanger.h

    std::unordered_set<uint64_t> _ignored_events;

    Server(Connection& conn);
};
