#include "container.h"
#include "logger.h"

void Layout_container::update_focus()
{
}

void Horizontal_container::update_rect(const Vector2D& rect)
{
    logger::debug("Updating Horizontal rect -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    Container::update_rect(rect);

    int next_pos_x         = rect.pos.x;
    const float multiplier = 1.0f / static_cast<float>(size());
    const int f_width      = (uint32_t)std::round(multiplier * (float)rect.size.x);

    for (const auto& child : _children) {
        child->update_rect({
            { next_pos_x, rect.pos.y  },
            { f_width,    rect.size.y }
        });
        next_pos_x += f_width;
    }
}

void Vertical_container::update_rect(const Vector2D& rect)
{
    logger::debug("Updating Vertical rect -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    Container::update_rect(rect);

    int next_pos_y         = rect.pos.y;
    const float multiplier = 1.0f / static_cast<float>(size());
    const int f_height     = (uint32_t)std::round(multiplier * (float)rect.size.x);

    for (const auto& child : _children) {
        child->update_rect({
            { rect.pos.x, next_pos_y }, 
            { rect.size.x, f_height  }
        });
        next_pos_y += f_height;
    }
}

void Tabbed_container::update_rect(const Vector2D& rect)
{
    logger::debug("Updating Vertical rect -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    Container::update_rect(rect);
    // Reduce height by tab size
    for (const auto& child : _children) {
        child->update_rect(rect);
    }
}

