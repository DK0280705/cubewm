#pragma once
#include <unordered_set>

extern "C" {
#include <xcb/xcb.h>
#include <xcb/xproto.h>
}

class Server;

class Event_handler
{
    std::unordered_set<uint64_t> _ignored_events;
    Server* _srv;

    // Handler functions
    void _key_press(const xcb_key_press_event_t& event);
    void _button_press(const xcb_button_press_event_t& event);
    void _motion_notify(const xcb_motion_notify_event_t& event);
    void _enter_notify(const xcb_enter_notify_event_t& event);
    void _leave_notify(const xcb_leave_notify_event_t& event);
    void _focus_in(const xcb_focus_in_event_t& event);
    void _focus_out(const xcb_focus_out_event_t& event);
    void _expose(const xcb_expose_event_t& event);
    void _destroy_notify(const xcb_destroy_notify_event_t& event);
    void _unmap_notify(const xcb_unmap_notify_event_t& event);
    void _map_request(const xcb_map_request_event_t& event);
    void _configure_notify(const xcb_configure_notify_event_t& event);
    void _configure_request(const xcb_configure_request_event_t& event);
    void _property_notify(const xcb_property_notify_event_t& event);
    void _selection_clear(const xcb_selection_clear_event_t& event);
    void _client_message(const xcb_client_message_event_t& event);
    void _mapping_notify(const xcb_mapping_notify_event_t& event);

public:
    Event_handler(Server* srv);

    void handle(int type, xcb_generic_event_t* event); 
};
