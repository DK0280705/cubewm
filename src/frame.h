#pragma once
#include "managed.h"
#include "container.h"

class Window;
class Layout;
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

class Layout_frame : public Managed<unsigned int>
                   , public Container
                   , public Focusable
{
    Layout& _layout;

public:
    Layout_frame(Index id, Layout& layout) noexcept
        : Managed(id)
        , _layout(layout)
    {}

    inline Layout& layout() const noexcept
    { return _layout; }
};

class Window_frame : public Managed<unsigned int>
                   , public Container
                   , public Focusable
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