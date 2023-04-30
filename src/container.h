#pragma once
/**
 * Explains the structure of the window manager
 * tiling, stacking, etc...
 * This is not like the container in STL
 * The container can be focused or not
 */
#include "geometry.h"

class Container
{
    Vector2D _rect;

public:
    Container() noexcept                   = default;
    Container(const Container&) noexcept   = delete;
    Container(Container&&) noexcept        = default;
    Container& operator=(const Container&) = delete;
    Container& operator=(Container&&)      = default;

    inline const Vector2D& rect() const noexcept
    { return _rect; }

    inline void rect(const Vector2D& rect) noexcept
    { _rect = rect; update_rect(); }

    virtual void update_rect() noexcept = 0;

    // A container is special, therefore no duplicates.
    // To be equal, means it's the same object.
    friend inline bool operator==(const Container& rhs, const Container& lhs) noexcept
    { return &rhs == &lhs; }
};