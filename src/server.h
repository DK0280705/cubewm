#pragma once
#include "xkb.h"
/**
 * Implements server for running everything
 * Not a god class, *sigh*, don't
 */

class Connection;
class State;

class Server
{
protected:
    State& _state;
    XKB*   _xkb;
    bool   _running;

public:
    static Server& instance();

    inline bool is_running() const noexcept
    { return _running; }

    inline auto xkb() const noexcept -> XKB&
    { return *_xkb; }

    virtual void start() = 0;
    virtual void stop()  = 0;

    virtual ~Server() noexcept;

protected:
    static Server* _instance;
    Server(Connection& conn) noexcept;
};
