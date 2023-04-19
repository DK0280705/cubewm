#pragma once
#include "geometry.h"
#include "managed.h"
#include "layout.h"
#include "keybind.h"

class State;

class Binding : public Managed<Keybind>
{
protected:
    Binding(const Index& k)
        : Managed(k)
    {}
public:
    virtual void operator()(State& state) const noexcept = 0;
    virtual ~Binding() = default;
};

class Move_focus : public Binding
{
    Direction _dir;
public:
    Move_focus(const Index& k, Direction dir) noexcept
        : Binding(k)
        , _dir(dir)
    {}

    void operator()(State& state) const noexcept override;
};

class Move_container : public Binding
{
    Direction _dir;
public:
    Move_container(const Index& k, Direction dir) noexcept
        : Binding(k)
        , _dir(dir)
    {}

    void operator()(State& state) const noexcept override;
};

class Change_layout_type : public Binding
{
    Layout::Type _change_to;
public:
    Change_layout_type(const Index& k, Layout::Type change_to) noexcept
        : Binding(k)
        , _change_to(change_to)
    {}

    void operator()(State& state) const noexcept override;
};

// If we need custom parameter so desperately, we can use buffer pattern