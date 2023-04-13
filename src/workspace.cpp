#include "workspace.h"
#include "window.h"
#include "logger.h"
#include <algorithm>

void Window_list::add(Window& window) noexcept
{
    assert_debug(!contains(window), "You can't add existing window to window list");
    logger::debug("Window list -> adding window: {:#x}", window.index());
    _list.push_back(&window);
    _pos.insert(&window);
}

void Window_list::focus(Const_iterator it) noexcept
{
    if (!current()) return;
    logger::debug("Window list -> focusing window: {:#x}", (*it)->index());
    current()->get().unfocus();
    auto* window = *it;
    _list.splice(_list.end(), _list, it);
    window->focus();
}

void Window_list::remove(Window& window) noexcept
{
    if (window.focused()) window.unfocus();

    logger::debug("Window list -> removing window: {:#x}", window.index());
    assert_debug(current(), "Attempted to remove foreign window");
    auto& last = current()->get();
    _list.remove(&window);
    _pos.erase(&window);

    if (&window == &last and current().has_value()) {
        logger::debug("Window list -> refocusing last window");
        focus(std::prev(_list.cend(), 1));
    }
}

void Workspace::update_rect() noexcept
{
    const auto& rect = this->rect();
    logger::debug("Workspace rect update -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    for (const auto& lcon : *this)
        lcon->rect(rect);
}
