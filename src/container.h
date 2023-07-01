#pragma once
/**
 * Explains the structure of the window manager
 * tiling, stacking, etc...
 * This is not like the container in STL
 * The container can be focused or not
 */
#include "geometry.h"

#include <cassert>

class Container
{
    Vector2D _rect;
    bool     _focused;

protected: // To avoid ambiguous name.
    virtual void _update_rect_fn()  noexcept = 0;
    virtual void _update_focus_fn() noexcept = 0;

public:
    Container() noexcept                   = default;
    Container(const Container&) noexcept   = delete;
    Container(Container&&) noexcept        = default;
    Container& operator=(const Container&) = delete;
    Container& operator=(Container&&)      = default;

    inline auto rect() const noexcept -> const Vector2D&
    { return _rect; }
    inline void rect(const Vector2D& rect) noexcept
    {
        _rect = rect;
        _update_rect_fn();
    }
    inline void update_rect() noexcept
    { _update_rect_fn(); }

    inline bool focused() const noexcept
    { return _focused; }
    inline void focus() noexcept
    {
        assert(!_focused);
        _focused = true;
        _update_focus_fn();
    }
    inline void unfocus() noexcept
    {
        assert(_focused);
        _focused = false;
        _update_focus_fn();
    }

    virtual ~Container() noexcept = default;
};

// A container is special, therefore no duplicates.
// To be equal, means it's the same object.
inline bool operator==(const Container& rhs, const Container& lhs) noexcept
{ return &rhs == &lhs; }