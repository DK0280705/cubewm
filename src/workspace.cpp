#include "workspace.h"
#include "window.h"
#include "logger.h"
#include <algorithm>

void Window_list::add(Window* win)
{
    assert_debug(!contains(win), "You can't add existing window to window list");
    logger::debug("Adding new focus");
    _list.push_back(win);
    _pos.insert(win);
}

void Window_list::focus(Const_iterator it)
{
    logger::debug("Focusing window on window list");
    current()->unfocus();
    auto* win = *it;
    _list.splice(_list.end(), _list, it);
    win->focus();
}

void Window_list::remove(Window* win)
{
    if (win->focused()) win->unfocus();

    logger::debug("Removing focus");
    auto* last = current();
    _list.remove(win);
    _pos.erase(win);

    if (win == last && current()) {
        logger::debug("Refocusing last focus");
        focus(std::prev(_list.cend(), 1));
    }
}

void Workspace::update_rect()
{
    const auto& rect = this->rect();
    logger::debug("Updating Workspace rect -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    for (const auto& lcon : *this)
        lcon->rect(rect);
}
