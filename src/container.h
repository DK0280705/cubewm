#pragma once
/**
 * Explains the structure of the window manager
 * tiling, stacking, etc...
 * This is not like the container in STL
 * The container can be focused or not
 */
#include "node.h"
#include "geometry.h"
#include <iterator>
#include <type_traits>
#include <vector>

class Container
{
    Vector2D _rect;

public:
    inline const Vector2D& rect() const noexcept
    { return _rect; }

    inline void rect(const Vector2D& rect) noexcept
    { _rect = rect; update_rect(); }

    virtual void update_rect() noexcept = 0;
};

class Layout_container : public Node<Container>
{
public:
    Layout_container() noexcept = default;

    virtual ~Layout_container()
    { for (const auto& c : *this) delete &c; };
};

class Horizontal_container : public Layout_container
{
public:
    Horizontal_container() noexcept = default;

    void update_rect() noexcept override;
};

class Vertical_container : public Layout_container
{
public:
    Vertical_container() noexcept = default;

    void update_rect() noexcept override;
};

class Tabbed_container : public Layout_container
{
public:
    Tabbed_container() noexcept = default;

    void update_rect() noexcept override;
};

