#include "monitor.h"

void Monitor::update_rect(const Vector2D& rect)
{
    Container::update_rect(rect);
    for (const auto& lcon : _children)
        lcon->update_rect(rect);
}
