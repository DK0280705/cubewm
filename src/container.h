#pragma once
#include "rect.h"
#include <list>

// Forward declarations
class Workspace; // #include "workspace.h"
struct Window;    // #include "window.h"

enum class CO
{
    Horizontal,
    Vertical
};

enum class CT
{
    Container,
    Dockarea,
    Workspace,
};

class Container
{
public:
    Container() noexcept;

    inline CT type() const
    { return _type; }

    inline CO orientation() const
    { return _orientation; }

    inline const Rectangle& rect() const
    { return _rect; }

    inline Container* parent() const
    { return _parent; }

    inline Container* front() const
    { return _children.front(); }

    inline Container* back() const
    { return _children.back(); }

    inline Container* at(int index) const
    { return *std::next(_children.begin(), index); }

    inline std::size_t size() const
    { return _children.size(); }

    inline bool empty() const
    { return _children.empty(); }

    virtual bool leaf() const
    { return false; }

    virtual Container* add_child(Container* con, Container* next_to = nullptr);
    
    virtual Container* transfer_child(Container* con);
    
    virtual Container* remove_child(Container* con);

    virtual Workspace* get_workspace();

    // Configure children rect
    void configure_child_rect();

    virtual ~Container();

protected:
    CT         _type;
    CO         _orientation;
    Rectangle  _rect;
    Container* _parent;

    std::list<Container*> _children;
};

class Window_container : public Container
{
public:
    Window_container(Window* win);

    inline Window* window() const
    { return _window; }

    bool leaf() const override
    { return true; }

private:
    Window* _window;
    // Fill with decorator/frame or etc
};
