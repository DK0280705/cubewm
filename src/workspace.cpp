#include "workspace.h"
#include "config.h"
#include "window.h"
#include "logger.h"
#include <algorithm>
#include <stdexcept>

void Workspace::_Window_list::add(Window& window) noexcept
{
    if (!empty() && current().focused())
        current().unfocus();
    _list.push_front(&window);
}

void Workspace::_Window_list::focus(const_iterator it) noexcept
{
    assert(!empty());
    if (_list.back()->focused())
        _list.back()->unfocus();

    Window& window = *it;
    _list.erase(it);
    _list.push_front(&window);
    window.focus();
}

void Workspace::_Window_list::remove(const_iterator it) noexcept
{
    if (it->focused())
        it->unfocus();

    _list.erase(it);
}

Workspace::Workspace(const Index id)
    : Managed(id)
    // by default the name is the id.
    , _name(std::to_string(id + 1))
{
    auto* floating_layout = new Layout(Layout::Containment_type::Floating);
    this->add_child(*floating_layout);
}

void Workspace::_update_rect_fn() noexcept
{
    const auto& rect = this->rect();
    logger::debug("Workspace rect update -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    for (auto& lcon : *this) {
        lcon.rect({
            { rect.pos.x + (int)config::GAP_SIZE, rect.pos.y + (int)config::GAP_SIZE },
            { rect.size.x - 2 * (int)config::GAP_SIZE, rect.size.y - 2 * (int)config::GAP_SIZE }
        });
    }
}

void Workspace::_update_focus_fn() noexcept
{
    if (focused()) {
        for (auto& window : _window_list) window.normalize();

        if (!_window_list.empty()) _window_list.current().focus();
    } else {
        if (!_window_list.empty()) _window_list.current().unfocus();

        for (auto& window : _window_list) window.minimize();
    }
}

void Workspace::add_window(Window& window) noexcept
{
    assert(std::ranges::find(_window_list, window) == _window_list.cend());
    _window_list.add(window);
    _window_list.current().focus();
}

void Workspace::focus_window(Window& window) noexcept
{
    if (window == _window_list.current()) {
        // Window just wanted to be refocused.
        if (window.focused()) window.unfocus();
        window.focus();
    } else if (window != _window_list.current()) {
        auto it = std::ranges::find(_window_list, window);
        assert(it != _window_list.cend());
        _window_list.focus(it);
    }
}

void Workspace::remove_window(Window& window) noexcept
{
    auto it = std::ranges::find(_window_list, window);
    assert(it != _window_list.cend());
    _window_list.remove(it);
}

Workspace::~Workspace() noexcept
{
    for (const auto& c : *this) delete &c;
}

namespace workspace {

} // namespace workspace
