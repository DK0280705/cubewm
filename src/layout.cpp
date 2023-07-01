#include "layout.h"
#include "logger.h"
#include "x86intrin.h"

static auto _reciprocal(float x) -> float
{
#if defined(__x86_64__) or defined(__i386__)
    float r = _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(x)));
    return r;
#else
    return 1/x;
#endif
}

Layout::Layout(Containment_type type)
    : _type(type)
{}

static void _update_rect_horizontal(Layout& layout)
{
    const auto& rect = layout.rect();
    logger::debug("Hcon rect update -> x: {}, y: {}, width: {}, height: {}",
                  rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);

    int next_pos_x    = rect.pos.x;
    const int f_width = std::round(_reciprocal((float)layout.size()) * (float)rect.size.x);
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
    const int f_height = std::round(_reciprocal((float)layout.size()) * (float)rect.size.y);
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

void Layout::_update_rect_fn() noexcept
{
    switch(_type) {
    case Containment_type::Horizontal:
        return _update_rect_horizontal(*this);
    case Containment_type::Vertical:
        return _update_rect_vertical(*this);
    case Containment_type::Tabbed:
        return _update_rect_tabbed(*this);
    case Containment_type::Floating:
        return; // Useless, size does not matter here.
    default: break; // silence
    }
}

void Layout::_update_focus_fn() noexcept
{
}

Layout::~Layout() noexcept
{
    for (const auto& child : *this) delete &child;
}
