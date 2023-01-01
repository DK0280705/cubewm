#pragma once
#include "rect.h"
#include <string>

// Forward Declarations
typedef unsigned int xcb_atom_t;
class Workspace;        // #include "workspace.h"
class Window_container; // #include "container.h"

struct Window
{
    using Window_id = unsigned int;

    Window_id   id;
    Rectangle   rect;
    xcb_atom_t  type;
    std::string name;
    std::string role;
    std::string wclass; // You can't really name it 'class'
    std::string winstance;

    Workspace*        workspace;
    Window_container* container;

    Window(Window_id id) noexcept;
};
