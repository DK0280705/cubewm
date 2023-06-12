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
    auto* floating_layout = new Layout(Layout::Type::Floating);
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