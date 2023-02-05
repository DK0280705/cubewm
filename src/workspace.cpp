#include "workspace.h"

void Workspace::update_focus()
{
}

void Workspace::update_rect(const Vector2D& rect)
{
    Container::update_rect(rect);
    for (const auto& lcon : _children)
        lcon->update_rect(rect);
}
