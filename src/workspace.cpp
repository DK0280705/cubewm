#include "workspace.h"
#include "config.h"
#include "window.h"
#include "logger.h"
#include <algorithm>
#include <stdexcept>

void Window_list::add(Window& window)
{
    logger::debug("Window list -> adding window: {:#x}", window.index());

    _list.push_back(&window);
}

void Window_list::focus(const_iterator it)
{
    logger::debug("Window list -> focusing window: {:#x}", (it->index()));

    if (_list.back()->focused()) _list.back()->unfocus();
    auto& window = *it;
    _list.erase(it);
    _list.push_back(&window);
    window.focus();
}

void Window_list::remove(const_iterator it)
{
    logger::debug("Window list -> removing window: {:#x}", it->index());

    if (it->focused()) it->unfocus();
    _list.erase(it);
}

Workspace::Workspace(const Index id)
    : Root<Container>()
    , Managed(id)
    // by default the name is the id.
    , _name(std::to_string(id))
    , _focused_layout(nullptr)
{
    Layout* floating_layout = new Layout(Layout::Type::Floating);
    this->add(*floating_layout);
}

void Workspace::update_rect() noexcept
{
    const auto& rect = this->rect();
    logger::debug("Workspace rect update -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    for (auto& lcon : *this) {
        lcon.rect({
            { rect.pos.x + (int)config::gap_size, rect.pos.y + (int)config::gap_size },
            { rect.size.x - 2 * (int)config::gap_size, rect.size.y - 2 * (int)config::gap_size }
        });
    }
}

void add_window(Window_list& window_list, Window& window)
{
    assert(&window.root<Workspace>().window_list() == &window_list);
    assert(std::ranges::find(window_list, window) == window_list.cend());
    window_list.add(window);
}

void focus_window(Window_list& window_list, Window& window)
{
    auto it = std::ranges::find(window_list, window);
    assert(it != window_list.cend());
    window_list.focus(it);
}

void focus_last(Window_list& window_list)
{
    if (!window_list.empty())
        window_list.focus(std::ranges::prev(window_list.cend()));
}

void try_focus_window(Window& window)
{
    auto& window_list = window.root<Workspace>().window_list();
    assert(!window_list.empty());
    if (window != window_list.last()->get()) {
        auto it = std::ranges::find(window_list, window);
        // This FAILS if try to focus before add_window().
        assert(it != window_list.cend());
        window_list.focus(it);
    }
}

void remove_window(Window_list& window_list, Window& window)
{
    auto it = std::ranges::find(window_list, window);
    assert(it != window_list.cend());
    window_list.remove(it);
}