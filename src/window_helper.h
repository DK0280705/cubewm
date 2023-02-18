#pragma once
#include <stdexcept>

class Container;
class Window;
class Workspace;
class Layout_container;

struct place
{
    Window* window;
    bool    create_new;
    place(Window* window, bool create_new = false) noexcept
        : window(window)
        , create_new(create_new)
    {}

    void operator()(Workspace& ws) const;
    void operator()(Layout_container& con) const;
};

struct purge
{
    Window* window;
    purge(Window* window) noexcept
        : window(window)
    {}

    void operator()(Layout_container& con) const;
};
