#pragma once
/**
 * Implements server for running everything
 * Not a god class, *sigh*, don't
 */

class State;

class Server
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

    virtual ~Server();

protected:
    Server(State& state);
};
