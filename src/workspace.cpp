#include "workspace.h"
#include "window.h"
#include "logger.h"
#include <algorithm>

void Focus_list::add(Window* win, bool focus)
{
    current() ? current()->focused() ? current()->unfocus()
                                     : void(0) 
              : void(0);
    
    if (_pos.contains(win)) {
        logger::debug("Adding exisiting focus");
        auto it = std::find(_list.begin(), _list.end(), win);
        _list.splice(_list.end(), _list, it);
    } else {
        logger::debug("Adding new focus");
        _list.push_back(win);
        _pos.insert(win);
    }
    if (focus) win->focus();
}

void Focus_list::remove(Window* win)
{
    auto* last = current();
    if (win->focused())
        win->unfocus();
    logger::debug("Removing focus");
    _list.remove(win);
    _pos.erase(win);
    if (win == last && current()) {
        logger::debug("Refocusing last focus");
        add(current());
    }
}

void Workspace::update_rect(const Vector2D& rect)
{
    logger::debug("Updating Workspace rect -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    Container::update_rect(rect);
    for (const auto& lcon : _children)
        lcon->update_rect(rect);
}
