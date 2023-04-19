#pragma once
#include "managed.h"

class Window;
class Focusable
{
protected:
    bool _focused{};

public:
    inline bool focused() const noexcept
    { return _focused; }

    virtual void focus() = 0;
    virtual void unfocus() = 0;
    virtual ~Focusable() noexcept = default;
};

class Window_frame : public Managed<unsigned int>
{
    Window& _window;

public:
    Window_frame(Index id, Window& window) noexcept
        : Managed(id)
        , _window(window)
    {}

    inline Window& window() const noexcept
    { return _window; }
};

class Container_frame : public Focusable
{

};