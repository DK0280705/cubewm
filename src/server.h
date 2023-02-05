#pragma once
/**
 * Implements server for running everything
 * Not a god class, *sigh*, don't
 */
#include "helper.h"

class Connection;
class State;

class Server : public Visit<Server>
{
protected:
    State& _state;
    bool   _stopping;

public:
    inline bool stopping() const
    { return _stopping; }

public:
    virtual void start() = 0;
    virtual void stop()  = 0;

protected:
    Server(Connection& conn);
};

namespace X11 {

class Event_handler;

class Server : public ::Server
             , public Init_once<Server>
{
    Event_handler& _ev;

public:
    Server(::Connection& conn);

    void start() override;
    void stop()  override;

private:
    void _main_loop();
};

}
