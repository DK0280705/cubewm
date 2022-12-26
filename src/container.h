#pragma once
#include "rect.h"
#include <list>
#include <string>

// Forward declarations
class Workspace; // #include "workspace.h"
class Window;    // #include "window.h"

class Container
{
public:
    enum
    {
        CO_HORIZONTAL,
        CO_VERTICAL
    } orientation;

    enum
    {
        CT_DOCKAREA,
        CT_WORKSPACE,
        CT_CONTAINER,
    } type;

    Rectangle   rect;
    std::string name;

    bool floating;
    bool hidden;
    
    Container() noexcept;

    inline Container* parent() const
    { return _parent; }

    inline std::size_t size() const
    { return _children.size(); }

    inline Container* front() const
    { return _children.front(); }

    inline Container* back() const
    { return _children.back(); }

    inline Container* at(int index) const
    { return *std::next(_children.begin(), index); }

    /**
     * @brief Add child to container
     * @param con container to add
     */
    virtual Container* add_child(Container* con);
    /**
     * @brief Transfer another container child to this container
     * @param con container to move
     */
    virtual Container* transfer_child(Container* con);
    /**
     * @brief Remove child from container
     * @param con container to remove
     */
    virtual Container* remove_child(Container* con);

    virtual Workspace* get_workspace();

    virtual ~Container();

protected:
    Container*            _parent;
    std::list<Container*> _children;
};

class Window_container : public Container
{
public:
    Window_container(Window* win);

private:
    Window* _window;
    // Fill with decorator/frame or etc
};
