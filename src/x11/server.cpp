#include "server.h"
#include "ewmh.h"
#include "event.h"
#include "monitor.h"
#include "window.h"
#include "xkb.h"

#include "../state.h"
#include "../connection.h"

#include <xcb/xcb.h>

namespace X11 {

Server::Server(::Connection& conn)
    : ::Server(State::init(conn, *this))
{
}

auto Server::init(::Connection& conn) -> Server&
{
    // Init X11 first, so we can call the xcb functions.
    X11::init(conn);
    auto& server = Init_once<X11::Server>::init(conn);
    auto& state  = server._state;
    X11::XKB::init(conn);

    window::grab_keys(X11::root_window_id(state.conn()), state);

    // Register emwh functions

    // Updating current workspace means updating current active window.
    state.connect<State::current_workspace_update>([](const State& state) {
        ewmh::update_net_current_desktop(state.current_workspace());
        const auto& window_list = state.current_workspace().window_list();
        uint32_t window_id = window_list.empty()
                           ? XCB_NONE
                           : window_list.current().index();
        ewmh::update_net_active_window(window_id);
    });
    state.connect<State::window_manager_update>(ewmh::update_net_client_list);
    state.connect<State::workspace_manager_update>(ewmh::update_net_desktop_names);
    state.connect<State::workspace_manager_update>(ewmh::update_net_number_of_desktops);
    state.notify_all();

    return server;
}

static void _main_loop(Server& server, State& state)
{
    xcb_generic_event_t* ev = nullptr;
    auto _ = memory::finally([&]() { if (ev) free(ev); });

    // Use file descriptor to get event
    const int xcb_fd = xcb_get_file_descriptor(state.conn());
    fd_set in_fds;
    FD_ZERO(&in_fds);
    FD_SET(xcb_fd, &in_fds);

    while (server.is_running()) {
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
            state.conn().flush();
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
