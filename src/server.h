#pragma once
#include "xkb.h"
/**
 * Implements server for running everything
 * Not a god class, *sigh*, don't
 */

class State;

class Server
{
protected:
    State& _state;
    bool   _running;

public:
    static auto instance() -> Server&;

    inline bool is_running() const noexcept
    { return _running; }

    virtual void start() = 0;
    virtual void stop()  = 0;

    virtual ~Server() noexcept;

protected:
    static Server* _instance;
    explicit Server(State& state) noexcept;
};
