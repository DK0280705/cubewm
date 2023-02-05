#pragma once
#include "workspace.h"
class Window;

struct Place_window
{
    Window* window;
    void operator ()(Manager<Workspace>& manager) const;
};

struct Purge_window
{
    Window* window;
    void operator ()(Manager<Workspace>& manager) const;
};
