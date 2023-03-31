#include "../state.h"
#include "../connection.h"
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
    X11::init(_state);
    _state.init_keyboard<X11::Keyboard>(_state.conn());
    // Get default workspace
    auto* workspc = _state.current_workspace();
    assert_debug(workspc, "Expected default workspace not null");

    // Default, will add randr soon
    _state.manager<Monitor>().accept([&](Manager<Monitor>& mgr) {
        if (mgr.empty())
            mgr.manage(0);
        Monitor* mon = mgr.at(0);
        const Vector2D rect = {
            { 0, 0 },
            { state.conn().xscreen()->width_in_pixels,
                state.conn().xscreen()->height_in_pixels }
        };
        mon->rect(rect);
    });

    X11::window::load_all(_state);
    auto& window_list = _state.current_workspace()->window_list();
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

        while ((ev = xcb_poll_for_event(_state.conn()))) {
            X11::event::handle({ ev });
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
