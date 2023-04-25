#include "layout.h"
#include "logger.h"

static constexpr int _fraction(std::size_t size, float length)
{
    const float frac = 1.0f / static_cast<float>(size);
    return (int)std::round(frac * length);
}

void Layout::update_rect() noexcept
{
    const auto& rect = this->rect();
    switch(_type) {
    case Type::Horizontal:
    {
        logger::debug("Hcon rect update -> x: {}, y: {}, width: {}, height: {}",
                       rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);

        int next_pos_x    = rect.pos.x;
        const int f_width = _fraction(size(), (float)rect.size.x);
        for (auto& child : *this) {
            child.rect({
                { next_pos_x, rect.pos.y  },
                { f_width,    rect.size.y }
            });
            next_pos_x += f_width;
        }
        break;
    }
    case Type::Vertical:
    {
        logger::debug("Vcon rect update -> x: {}, y: {}, width: {}, height: {}",
                      rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);

        int next_pos_y     = rect.pos.y;
        const int f_height = _fraction(size(), (float)rect.size.y);
        for (auto& child : *this) {
            child.rect({
                { rect.pos.x, next_pos_y },
                { rect.size.x, f_height  }
            });
            next_pos_y += f_height;
        }
        break;
    }
    case Type::Tabbed:
    {
        logger::debug("Tcon rect update -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);

        // Reduce height by tab size
        for (auto& child : *this) {
            child.rect(rect);
        }
        break;
    }
    case Type::Floating:
        // Useless, size does not matter here.
    default:
        break;
    }
}