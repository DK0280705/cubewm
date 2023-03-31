#pragma once
#include "geometry.h"

class State;

class Command
{
protected:
    Command();
public:
    virtual void operator()(State& state);
    virtual ~Command() = default;
};

class Move_focus : public Command
{
    direction _dir;
public:
    Move_focus(direction dir) noexcept
        : _dir(dir)
    {}

    void operator()(State& state) override;
};

// If we need custom parameter so desperately, we can use buffer pattern