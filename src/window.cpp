#include "window.h"

Window::Window(Window_id id) noexcept
    : id(id)
    , rect({0, 0, 0, 0})
    , type(0)
    , workspace(nullptr)
    , container(nullptr)
{
}
