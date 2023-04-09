#include "../state.h"
#include "../connection.h"
#include "ewmh.h"
#include "keyboard.h"
#include "server.h"
#include "event.h"
#include "window.h"
#include "x11.h"
#include <xcb/xcb.h>

namespace X11 {

Server::Server(State& state)
    : ::Server(state)
{
    // Init X11 first, so we can call the xcb functions.
    X11::init(_state.conn());

    // Keyboard must be initialized after xkb extension request.
    _state.init_keyboard<X11::Keyboard>(_state.conn());

    window::grab_keys(_state.keyboard(), _state.conn().xscreen()->root);

    // Register emwh functions
    _state.connect(State::current_monitor_update, ewmh::update_current_desktop);
    _state.connect(State::window_manager_update, ewmh::update_client_list);
    _state.connect(State::workspace_manager_update, ewmh::update_desktop_names);
    _state.connect(State::workspace_manager_update, ewmh::update_number_of_desktops);
    _state.notify_all();

    auto& mon_mgr = _state.manager<::Monitor>();
    // Default, will add randr soon
    auto* xscreen = _state.conn().xscreen();
    mon_mgr.at(0)->rect({
        { 0, 0 },
        { xscreen->width_in_pixels, xscreen->height_in_pixels }
    });

    // Get default workspace
    auto* workspc = _state.current_workspace();
    assert_debug(workspc, "Expected default workspace not null");

    // <Manage all existing windows if available.
    X11::window::load_all(_state);
    // Make current workspace last added window focused.
    auto& window_list = workspc->window_list();
    if (window_list.current())
        window_list.focus(std::prev(window_list.end(), 1));
}

void Server::_main_loop()
{
    xcb_generic_event_t* ev = nullptr;
    auto _ = finally([&]() { if (ev) free(ev); });
    const int xcb_fd = xcb_get_file_descriptor(_state.conn());
    fd_set in_fds;

    while (!_stopping) {
        // Use file descriptor to get event
        FD_ZERO(&in_fds); //NOLINT
        FD_SET(xcb_fd, &in_fds); //NOLINT

        // Freezes until signal
        select(xcb_fd + 1, &in_fds, nullptr, nullptr, nullptr);
        if (xcb_connection_has_error(_state.conn())) {
            stop();
            continue;
        }

        while ((ev = xcb_poll_for_event(_state.conn()))) {
            X11::event::handle(_state, { ev });
            free(ev);
            ev = nullptr;
            xcb_flush(_state.conn());
        }
    }
}

void Server::start()
{
    _main_loop();
}

void Server::stop()
{
    _stopping = true;
}

}
