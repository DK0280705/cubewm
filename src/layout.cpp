#include "layout.h"
#include "logger.h"
#include "x11/frame.h"

static constexpr int _fraction(std::size_t size, float length)
{
    const float frac = 1.0f / static_cast<float>(size);
    return (int)std::round(frac * length);
}

Layout::Layout(Type type)
    : _type(type)
    #ifndef USE_WAYLAND
    , _frame(new X11::Layout_frame(*this))
    #endif
{}

static void _update_rect_horizontal(Layout& layout)
{
    const auto& rect = layout.rect();
    logger::debug("Hcon rect update -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);

    int next_pos_x    = rect.pos.x;
    const int f_width = _fraction(layout.size(), (float)rect.size.x);
    for (auto& child : layout) {
        child.rect({
            { next_pos_x, rect.pos.y  },
            { f_width,    rect.size.y }
        });
        next_pos_x += f_width;
    }
}

static void _update_rect_vertical(Layout& layout)
{
    const auto& rect = layout.rect();
    logger::debug("Vcon rect update -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);

    int next_pos_y     = rect.pos.y;
    const int f_height = _fraction(layout.size(), (float)rect.size.y);
    for (auto& child : layout) {
        child.rect({
            { rect.pos.x, next_pos_y },
            { rect.size.x, f_height  }
        });
        next_pos_y += f_height;
    }
}

static void _update_rect_tabbed(Layout& layout)
{
    const auto& rect = layout.rect();
    logger::debug("Tcon rect update -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);

    // Reduce height by tab size
    for (auto& child : layout) {
        child.rect(rect);
    }
}

void Layout::update_rect() noexcept
{
    switch(_type) {
    case Type::Horizontal:
        return _update_rect_horizontal(*this);
    case Type::Vertical:
        return _update_rect_vertical(*this);
    case Type::Tabbed:
        return _update_rect_tabbed(*this);
    case Type::Floating:
        return; // Useless, size does not matter here.
    default: break; // silence
    }
}