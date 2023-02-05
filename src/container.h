#pragma once
/**
 * Explains the structure of the window manager
 * tiling, stacking, etc...
 */
#include "node.h"
#include "geometry.h"
#include <vector>

class Workspace;

class Container
{
    Vector2D         _rect;
    Node<Container>* _parent;

public:
    inline const Vector2D& rect() const
    { return _rect; }

    inline Node<Container>* parent() const
    { return _parent; }

    inline void parent(Node<Container>* parent)
    { _parent = parent; }

public:
    virtual void update_rect(const Vector2D& rect)
    { _rect = rect; }

    virtual void update_focus() = 0;

    virtual ~Container() {}
};

class Layout_container : public Node<Container>
{
    Workspace* _ws;
    std::vector<Container*> _focus_stack;

public:
    inline Workspace* workspace() const
    { return _ws; }

    inline void add_focus(Container* con)
    { _focus_stack.push_back(con); }

public:
    Layout_container(Workspace* ws) noexcept
        : _ws(ws)
    {}
    
    void update_focus() override;

    virtual ~Layout_container()
    { for (const auto& c : _children) delete c; };
};

class Horizontal_container : public Layout_container
{
public:
    Horizontal_container(Workspace* ws)
        : Layout_container(ws)
    {}

    void update_rect(const Vector2D& rect) override;
}; 

class Vertical_container : public Layout_container
{
public:
    Vertical_container(Workspace* ws)
        : Layout_container(ws)
    {}

    void update_rect(const Vector2D& rect) override;
};

class Tabbed_container : public Layout_container
{
public:
    Tabbed_container(Workspace* ws)
        : Layout_container(ws)
    {}

    void update_rect(const Vector2D& rect) override;
};

