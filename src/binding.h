#pragma once
#include "geometry.h"
#include "managed.h"
#include "layout.h"
#include "keybind.h"

class State;

class Binding : public Managed<Keybind>
{
protected:
    explicit Binding(const Index& k)
        : Managed(k)
    {}
public:
    virtual void execute(State& state) const noexcept = 0;
};

namespace binding {
class Move_focus : public Binding
{
    Direction _dir;
public:
    Move_focus(const Index& k, Direction dir) noexcept
        : Binding(k)
        , _dir(dir)
    {}

    void execute(State& state) const noexcept override;
};

class Move_container : public Binding
{
    Direction _dir;
public:
    Move_container(const Index& k, Direction dir) noexcept
        : Binding(k)
        , _dir(dir)
    {}

    void execute(State& state) const noexcept override;
};

class Change_layout_type : public Binding
{
    Layout::Containment_type _change_to;
public:
    Change_layout_type(const Index& k, Layout::Containment_type change_to) noexcept
        : Binding(k)
        , _change_to(change_to)
    {}

    void execute(State& state) const noexcept override;
};

class Switch_workspace : public Binding
{
    uint32_t _workspace_id;
public:
    Switch_workspace(const Index& k, uint32_t index) noexcept
        : Binding(k)
        , _workspace_id(index)
    {}

    void execute(State& state) const noexcept override;
};
} //namespace binding

// If we need custom parameter so desperately, we can use buffer pattern