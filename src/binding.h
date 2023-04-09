#pragma once
#include "geometry.h"
#include "managed.h"
#include "keybind.h"

class State;

class Binding : public Managed<Keybind>
{
protected:
    Binding(const Index& k)
        : Managed(k)
    {}
public:
    virtual void operator()(State& state) const = 0;
    virtual ~Binding() = default;
};

class Move_focus : public Binding
{
    direction _dir;
public:
    Move_focus(const Index& k, direction dir) noexcept
        : Binding(k)
        , _dir(dir)
    {}

    void operator()(State& state) const override;
};

// If we need custom parameter so desperately, we can use buffer pattern