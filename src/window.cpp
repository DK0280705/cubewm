#include "window.h"

Window::Window(Window_id id)
    : rect({0, 0, 0, 0})
    , fullscreen(false)
    , hidden(true)
    , _id(id)
{
}

Window::~Window()
{
}
