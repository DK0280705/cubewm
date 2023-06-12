#include "server.h"
#include "state.h"
#include <csignal>

Server* Server::_instance = nullptr;

static void _catch_signal(int) noexcept
{
    Server::instance().stop();
}

Server::Server(State& state) noexcept
    : _state(state)
    , _running(false)
{
    // Handle exit
    _instance = this;
    std::signal(SIGINT, _catch_signal);
    std::signal(SIGQUIT, _catch_signal);
    std::signal(SIGTERM, _catch_signal);
    std::signal(SIGCHLD, [](int){});
}

auto Server::instance() -> Server&
{
    assert(_instance);
    return *_instance;
}

Server::~Server() noexcept = default;
