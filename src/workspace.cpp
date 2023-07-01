#include "workspace.h"
#include "config.h"
#include "window.h"
#include "logger.h"
#include <algorithm>
#include <stdexcept>

Workspace::Workspace(const Index id)
    : Managed(id)
    // by default the name is the id.
    , _name(std::to_string(id))
    , _focused_layout(nullptr)
{
    auto* floating_layout = new Layout(Layout::Containment_type::Floating);
    this->add(*floating_layout);
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
        if (!_window_list.empty())
            _window_list.current().focus();
    } else {
        if (!_window_list.empty())
            _window_list.current().unfocus();
    }
}

Workspace::~Workspace() noexcept
{
    for (const auto& c : *this) delete &c;
}
