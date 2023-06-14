#pragma once
#include "managed.h"
#include "container.h"

class Window;
class Layout;

class Layout_frame : public Managed<unsigned int>
                   , public Container
{
    Layout& _layout;

public:
    Layout_frame(Index id, Layout& layout) noexcept
        : Managed(id)
        , _layout(layout)
    {}

    inline auto layout() const noexcept -> Layout&
    { return _layout; }
};

class Window_frame : public Managed<unsigned int>
                   , public Container
{
    Window& _window;

public:
    Window_frame(Index id, Window& window) noexcept
        : Managed(id)
        , _window(window)
    {}

    inline auto window() const noexcept -> Window&
    { return _window; }
};