#include "monitor.h"
#include "logger.h"
#include "workspace.h"

void Monitor::_update_rect_fn() noexcept
{
    const auto& rect = this->rect();
    logger::debug("Monitor rect update -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    // Should count for dockarea rect
    for (auto& ws : *this)
        ws.rect(rect);
}

void Monitor::_update_focus_fn() noexcept
{
    if (focused()) {
        if (_current) _current->focus();
    } else {
        if (_current) _current->unfocus();
    }
}

void Monitor::add(Workspace& workspace)
{
    workspace._monitor = this;
    _workspaces.push_back(&workspace);
}

void Monitor::remove(Monitor::const_iterator it)
{
    assert(std::ranges::contains(_workspaces, &*it));
    it->_monitor = nullptr;
    _workspaces.erase(it);
}

Monitor::~Monitor() noexcept
{
    { for (const auto& ws : _workspaces) delete ws; }
}
