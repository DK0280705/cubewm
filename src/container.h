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

struct container_visitor
{
    virtual void operator()(class Monitor&)              const noexcept {};
    virtual void operator()(class Workspace&)            const noexcept {};
    virtual void operator()(class Horizontal_container&) const noexcept {};
    virtual void operator()(class Vertical_container&)   const noexcept {};
    virtual void operator()(class Tabbed_container&)     const noexcept {};
    virtual void operator()(class Window&)               const noexcept {};
};

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
    virtual void accept(const container_visitor&) noexcept = 0;

    // A container is special, therefore no duplicates.
    // To be equal, means it's the same object.
    friend inline bool operator==(const Container& rhs, const Container& lhs) noexcept
    { return &rhs == &lhs; }
};

class Layout_container : public Node<Container>
{
protected:
    Layout_container() noexcept = default;

public:
    virtual ~Layout_container()
    { for (const auto& c : *this) delete &c; };
};

class Horizontal_container : public Layout_container
{
public:
    Horizontal_container() noexcept = default;

    void update_rect() noexcept override;
    void accept(const container_visitor& visitor) noexcept override
    { visitor(*this); }
};

class Vertical_container : public Layout_container
{
public:
    Vertical_container() noexcept = default;

    void update_rect() noexcept override;
    void accept(const container_visitor& visitor) noexcept override
    { visitor(*this); }
};

class Tabbed_container : public Layout_container
{
public:
    Tabbed_container() noexcept = default;

    void update_rect() noexcept override;
    void accept(const container_visitor& visitor) noexcept override
    { visitor(*this); }
};

