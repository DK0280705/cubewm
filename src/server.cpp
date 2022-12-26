#include "server.h"
#include "error.h"
#include "windowmanager.h"
#include "monitormanager.h"
#include "workspacemanager.h"
#include <csignal>

Server::Server(Connection& conn)
    : _conn(conn)
    , _terminating(false)
    , _window_manager(Window_manager::init<Window_manager>(*this))
    , _monitor_manager(Monitor_manager::init<Monitor_manager>(*this))
    , _workspace_manager(Workspace_manager::init<Workspace_manager>(*this))
{
    Workspace* first_ws = _workspace_manager.manage(0);
    _monitor_manager.place_workspace(0, first_ws);
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
