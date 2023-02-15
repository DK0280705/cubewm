#include "server.h"
#include "state.h"
#include <csignal>

// Deal with C
static Server* _psrv;

static void catch_signal(int)
{
    _psrv->stop();
}

Server::Server(Connection& conn)
    : _state(State::init(conn))
    , _stopping(false)
{
    _psrv = this;
    std::signal(SIGINT, catch_signal);
    std::signal(SIGQUIT, catch_signal);
    std::signal(SIGTERM, catch_signal);
    std::signal(SIGCHLD, [](int){});
}
