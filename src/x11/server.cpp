#include "../server.h"
#include "../state.h"
#include "../connection.h"
#include "event_handler.h"
#include "window.h"
#include <xcb/xcb.h>

namespace X11 {

Server::Server(::Connection& conn)
    : ::Server(conn)
    , _ev(Event_handler::init(_state))
{   
    auto& wor_mgr = _state.manager<Workspace>();
    auto* workspc = wor_mgr.manage(0);
    wor_mgr.set_current(0);

    _state.manager<Monitor>().accept([&](Manager<Monitor>& mgr) {
        if (mgr.empty()) {
            Monitor* mon = mgr.manage(0);
            mon->add(workspc);
            const Vector2D rect = {
                { 0, 0 },
                { conn.xscreen()->width_in_pixels, conn.xscreen()->height_in_pixels }
            };
            mon->update_rect(rect);
        }
    });

    X11::window::load_all(_state);
}

void Server::_main_loop()
{
    xcb_generic_event_t* event = nullptr;
    auto _ = finally([&]() { if (event) free(event); });
    const int xcb_fd = xcb_get_file_descriptor(_state.conn());
    fd_set in_fds;

    while (!_stopping) {
        // Use file descriptor to get event
        FD_ZERO(&in_fds); //NOLINT
        FD_SET(xcb_fd, &in_fds); //NOLINT

        // Freezes until signal
        select(xcb_fd + 1, &in_fds, nullptr, nullptr, nullptr);

        while ((event = xcb_poll_for_event(_state.conn()))) {
            _ev.handle({ event });
            free(event);
            event = nullptr;
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
