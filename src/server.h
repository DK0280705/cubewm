#pragma once
#include "atoms.h"
#include "rect.h"
#include <cstdint>
#include <span>
#include <string>
#include <unordered_set>

extern "C" {
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xproto.h>
}

class Root;
class Container;

enum XCursor : unsigned int
{
    XCURSOR_POINTER = 0,
    XCURSOR_RESIZE_HORIZONTAL,
    XCURSOR_RESIZE_VERTICAL,
    XCURSOR_TOP_LEFT_CORNER,
    XCURSOR_TOP_RIGHT_CORNER,
    XCURSOR_BOTTOM_LEFT_CORNER,
    XCURSOR_BOTTOM_RIGHT_CORNER,
    XCURSOR_WATCH,
    XCURSOR_MOVE,
    XCURSOR_MAX
};

class Server
{
    xcb_connection_t* conn;

public:
    Server(const Server& s)  = delete;
    Server(const Server&& s) = delete;

    ~Server();

    static Server& instance();

    int             default_screen;
    xcb_screen_t*   screen;
    xcb_timestamp_t timestamp;

    // Contains main loop
    void run();

    inline xcb_connection_t* operator()() const;
    inline xcb_window_t      root_window() const;
    inline Rectangle         rect() const;

    xcb_atom_t  atom(const std::string& atom_name) const;
    std::string atom_name(xcb_atom_t atom) const;

    void handle(int type, xcb_generic_event_t* event);
    void manage_window(xcb_window_t id);

    template<class T, std::size_t N>
    inline void change_property(
        xcb_window_t window, xcb_atom_t property, xcb_atom_t type, uint8_t format, std::span<T, N> data) const;
    inline void change_atom_property(xcb_window_t window, xcb_atom_t property, std::span<uint32_t> data) const;
    inline void change_string_property(xcb_window_t window, xcb_atom_t property, std::span<const char*> data) const;
    inline void change_window_property(xcb_window_t window, xcb_atom_t property, std::span<uint32_t> data) const;
    inline void change_cardinal_property(xcb_window_t window, xcb_atom_t property, std::span<uint32_t> data) const;

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

private:
    Server();

    Root*                        root;
    std::unordered_set<uint64_t> ignored_events;

    void acquire_timestamp();
};

xcb_connection_t* Server::operator()() const
{
    return conn;
}

xcb_window_t Server::root_window() const
{
    return screen->root;
}

Rectangle Server::rect() const
{
    return {0, 0, screen->width_in_pixels, screen->height_in_pixels};
}

template<class T, std::size_t N>
void Server::change_property(
    xcb_window_t window, xcb_atom_t property, xcb_atom_t type, uint8_t format, std::span<T, N> data) const
{
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, property, type, format, data.size(), data.data());
}

void Server::change_atom_property(xcb_window_t window, xcb_atom_t property, std::span<uint32_t> data) const
{
    change_property(window, property, XCB_ATOM_ATOM, 32, data);
}

void Server::change_string_property(xcb_window_t window, xcb_atom_t property, std::span<const char*> data) const
{
    change_property(window, property, UTF8_STRING, 8, data);
}

void Server::change_window_property(xcb_window_t window, xcb_atom_t property, std::span<uint32_t> data) const
{
    change_property(window, property, XCB_ATOM_WINDOW, 32, data);
}

void Server::change_cardinal_property(xcb_window_t window, xcb_atom_t property, std::span<uint32_t> data) const
{
    change_property(window, property, XCB_ATOM_CARDINAL, 32, data);
}
