#include "server.h"
#include "error.h"
#include "windowmanager.h"
#include "monitormanager.h"
#include "workspacemanager.h"
#include "xwrap.h"
#include <csignal>

static void manage_existing_windows(Window_manager& wm, Workspace_manager& wsm)
{
    const auto windows = XWrap::get_windows();
    for (const auto& win : windows) {
        if (window_manageable(wm, win, true)) {
            Window* w = wm.manage(win);
            const uint32_t ws_id = XWrap::get_window_workspace(w->id);
            if (!wsm.is_managed(ws_id))
                w->workspace = wsm.manage(ws_id);
            else w->workspace = wsm.at(ws_id);
            Window_container* con = new Window_container(w);
            wsm.place_container(wsm.current(), con);
        }
    }
}

Server::Server(Connection& conn)
    : _conn(conn)
    , _terminating(false)
    , _window_manager(Window_manager::init<Window_manager>(*this))
    , _monitor_manager(Monitor_manager::init<Monitor_manager>(*this))
    , _workspace_manager(Workspace_manager::init<Workspace_manager>(*this))
{
    _monitor_manager.update();
    Workspace* first_ws = _workspace_manager.manage(0);
    // Add workspace on primary monitor
    _monitor_manager.place_workspace(0, first_ws);
    _workspace_manager.focus(first_ws);

    manage_existing_windows(_window_manager, _workspace_manager);
}

Server& Server::init(Connection& conn)
{
    static Server srv(conn);
    assert_init(srv);
    return srv;
}

void Server::start()
{
    xcb_generic_event_t* event = nullptr;
    const int xcb_fd = xcb_get_file_descriptor(_conn);
    fd_set in_fds;

    while (!_terminating) {
        // Use file descriptor to get event
        FD_ZERO(&in_fds); //NOLINT
        FD_SET(xcb_fd, &in_fds); //NOLINT

        // Freezes until signal
        select(xcb_fd + 1, &in_fds, nullptr, nullptr, nullptr);

        xcb_flush(_conn);
        while ((event = xcb_poll_for_event(_conn))) {
            const int type = event->response_type & 0x7F;
            handle(type, event);
            free(event);

            xcb_flush(_conn);
        }
    }
}

void Server::stop()
{
    _terminating = true;
}

Server::~Server()
{
}
