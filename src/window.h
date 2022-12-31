#pragma once
#include "rect.h"
#include <string>

// Forward Declarations
class Window_container; // #include "container.h"

struct Window
{
    using Window_id = unsigned int;
    Window_id         id;
    Rectangle         rect;
    std::string       name;
    Window_container* container;

    Window(Window_id id) noexcept;
};
