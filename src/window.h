#pragma once
#include "rect.h"
#include <string>

// Forward Declarations
class Container;        // #include "container.h"
class Window_container; // ---------------------

class Window
{
public:
    using Window_id = unsigned int;

    Rectangle rect;

    std::string name;
    std::string role;

    bool fullscreen;
    bool hidden;

    inline Window_container* container() const
    { return _container; }

    Window(Window_id id);
    Window(const Window&)         = delete;
    Window(Window&&)              = delete;
    auto operator=(const Window&) = delete;
    auto operator=(Window&&)      = delete;
    ~Window();

private:
    Window_id _id;

    friend class Window_container;
    Window_container* _container;
};
