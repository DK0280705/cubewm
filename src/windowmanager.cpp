#include "windowmanager.h"
#include "logger.h"
#include "server.h"
#include "window.h"
#include "xwrap.h"
#include <algorithm>

Window_manager::Window_manager(Server& srv)
    : Manager(srv)
{
}

Window* Window_manager::manage(const Window_id id)
{
    Window* win = new Window(id);
    win->type   = XWrap::get_window_type(id);
    win->name   = XWrap::get_window_name(id);

    std::pair<std::string, std::string> i_c = XWrap::get_window_class(id);
    win->winstance = i_c.first;
    win->wclass    = i_c.second;

    _managed.emplace(id, win);
    return win;
}

void Window_manager::unmanage(const Window_id id)
{
    delete _managed.at(id);
    _managed.erase(id);
}

bool window_manageable(Window_manager& wm, Window_manager::Window_id id, bool must_be_mapped)
{
    if (wm.is_managed(id)) {
        Log::debug("Can't manage already managed window");
        return false;
    }

    auto attr = XWrap::get_window_attributes(id);
    
    if (attr->override_redirect) {
        Log::debug("Can't manage window with override_redirect attribute");
        return false;
    }

    return true;
}
