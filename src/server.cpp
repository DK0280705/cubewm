#include "server.h"
#include "state.h"
#include <csignal>

Server* Server::_instance = 0;

static void _catch_signal(int) noexcept
{
    Server::instance().stop();
}

Server::Server(Connection& conn) noexcept
    : _state(State::init(conn))
    , _running(false)
{
    // Handle exit
    _instance = this;
    std::signal(SIGINT, _catch_signal);
    std::signal(SIGQUIT, _catch_signal);
    std::signal(SIGTERM, _catch_signal);
    std::signal(SIGCHLD, [](int){});
}

Server& Server::instance()
{
    assert(_instance);
    return *_instance;
}

Server::~Server() noexcept
{
}
