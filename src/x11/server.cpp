#include "../state.h"
#include "../connection.h"
#include "ewmh.h"
#include "server.h"
#include "event.h"
#include "window.h"
#include "xkb.h"
#include <xcb/xcb.h>

namespace X11 {

Server::Server(::Connection& conn)
    : ::Server(conn)
{
}

Server& Server::init(::Connection& conn)
{
    // Init X11 first, so we can call the xcb functions.
    X11::init(conn);
    auto& server = Init_once<X11::Server>::init(conn);
    auto& state  = server._state;
    server._xkb  = &X11::XKB::init(conn);

    window::grab_keys(root_window_id(state.conn()), state);

    auto& mon_mgr = state.monitors();
    // Default, will add randr soon
    auto* xscreen = state.conn().xscreen();
    mon_mgr.at(0).rect({
        { 0, 0 },
        { xscreen->width_in_pixels, xscreen->height_in_pixels }
    });

    // Manage all existing windows if available.
    X11::window::load_all(state);
    // Make current workspace last added window focused.
    focus_last(state.current_workspace().window_list());
    // Register emwh functions
    state.connect<State::current_workspace_update>(ewmh::update_current_desktop);
    state.connect<State::window_manager_update>(ewmh::update_client_list);
    state.connect<State::workspace_manager_update>(ewmh::update_desktop_names);
    state.connect<State::workspace_manager_update>(ewmh::update_number_of_desktops);
    state.notify_all();

    return server;
}

static void _main_loop(Server& server, State& state)
{
    xcb_generic_event_t* ev = nullptr;
    auto _ = finally([&]() { if (ev) free(ev); });
    const int xcb_fd = xcb_get_file_descriptor(state.conn());
    fd_set in_fds;

    while (server.is_running()) {
        // Use file descriptor to get event
        FD_ZERO(&in_fds); //NOLINT
        FD_SET(xcb_fd, &in_fds); //NOLINT

        // Freezes until signal
        select(xcb_fd + 1, &in_fds, nullptr, nullptr, nullptr);
        if (xcb_connection_has_error(state.conn())) {
            server.stop();
            continue;
        }

        while ((ev = xcb_poll_for_event(state.conn()))) {
            X11::event::handle(state, { ev });
            free(ev);
            ev = nullptr;
            xcb_flush(state.conn());
        }
    }
}

void Server::start()
{
    if (_running) throw Server_error("Server already started");
    _running = true;
    _main_loop(*this, _state);
}

void Server::stop()
{
    if (!_running) throw Server_error("Server already stopped");
    _running = false;
}

}