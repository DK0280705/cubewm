#include "workspace.h"
#include "window.h"
#include "logger.h"
#include <algorithm>

void Window_list::add(Window* window)
{
    assert_debug(!contains(window), "You can't add existing window to window list");
    logger::debug("Window list -> adding window: {:#x}", window->index());
    _list.push_back(window);
    _pos.insert(window);
}

void Window_list::focus(Const_iterator it)
{
    logger::debug("Window list -> focusing window: {:#x}", (*it)->index());
    current()->unfocus();
    auto* window = *it;
    _list.splice(_list.end(), _list, it);
    window->focus();
}

void Window_list::remove(Window* window)
{
    if (window->focused()) window->unfocus();

    logger::debug("Window list -> removing window: {:#x}", window->index());
    auto* last = current();
    _list.remove(window);
    _pos.erase(window);

    if (window == last && current()) {
        logger::debug("Window list -> refocusing last window");
        focus(std::prev(_list.cend(), 1));
    }
}

void Workspace::update_rect()
{
    const auto& rect = this->rect();
    logger::debug("Workspace rect update -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    for (const auto& lcon : *this)
        lcon->rect(rect);
}
