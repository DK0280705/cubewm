#include "monitor.h"

void Monitor::update_rect()
{
    // Should count for dockarea rect
    for (const auto& ws : *this)
        ws->rect(rect());
}
