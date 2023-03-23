#pragma once
/**
 * Explains the structure of the window manager
 * tiling, stacking, etc...
 * This is not like the container in STL
 * The container can be focused or not
 */
#include "node.h"
#include "geometry.h"
#include <vector>

class Workspace;

class Container
{
    Vector2D _rect;

public:
    inline const Vector2D& rect() const
    { return _rect; }

    inline void rect(const Vector2D& rect)
    { 
        _rect = rect;
        update_rect();
    }

public:
    virtual void update_rect() = 0;

    virtual ~Container() {}
};

class Layout_container : public Node<Container>
{
    Workspace* _ws;

public:
    inline Workspace* workspace() const
    { return _ws; }

    inline void workspace(Workspace* ws)
    { _ws = ws; }

public:
    Layout_container(Workspace* ws) noexcept
        : _ws(ws)
    {}
   
    virtual ~Layout_container()
    { for (const auto& c : *this) delete c; };
};

class Horizontal_container : public Layout_container
{
public:
    Horizontal_container(Workspace* ws)
        : Layout_container(ws)
    {}

    void update_rect() override;
}; 

class Vertical_container : public Layout_container
{
public:
    Vertical_container(Workspace* ws)
        : Layout_container(ws)
    {}

    void update_rect() override;
};

class Tabbed_container : public Layout_container
{
public:
    Tabbed_container(Workspace* ws)
        : Layout_container(ws)
    {}

    void update_rect() override;
};

