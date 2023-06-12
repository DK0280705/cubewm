#include "monitor.h"
#include "logger.h"
#include "workspace.h"

void Monitor::update_rect() noexcept
{
    const auto& rect = this->rect();
    logger::debug("Monitor rect update -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    // Should count for dockarea rect
    for (auto& ws : *this)
        ws.rect(rect);
}

Monitor::~Monitor() noexcept
{
    { for (const auto& ws : _workspaces) delete ws; }
}