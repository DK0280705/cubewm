#include "windowmanager.h"
#include "server.h"
#include "window.h"
#include <algorithm>

Window_manager::Window_manager(Server& srv)
    : Manager(srv)
{
}

Window* Window_manager::manage(const Window_id id)
{
    Window* win = new Window(id);
    _managed.emplace(id, win);
    return win;
}

void Window_manager::unmanage(const Window_id id)
{
    delete _managed.at(id);
    _managed.erase(id);
}
