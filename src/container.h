#pragma once
#include "rect.h"
#include <list>
#include <string>

/**
 * Heavily inspired from i3 window manager's data structure
 * Which implements Root -> Outputs -> Workspaces/Dockareas -> Containers
 */
class Server;
class Workspace;
class Cube_window;

class Container
{
protected:
    Server& srv;

public:
    Container(Server& srv);
    virtual ~Container();

    Container* parent;

    enum
    {
        CO_HORIZONTAL,
        CO_VERTICAL
    } orientation;

    enum
    {
        CT_DOCKAREA,
        CT_WORKSPACE,
        CT_SPLIT,
        CT_CONTAINER,
    } type;

    Rectangle rect;

    Cube_window* window;

    std::string name;

    bool floating;

    inline bool is_leaf() { return children.empty() && window; }

    inline bool is_split() { return !is_leaf() && !type; }

    /**
     * @brief Add child to container
     * @param con container to add
     */
    virtual Container& add_child(Container* con);
    /**
     * @brief Transfer another container child to this container
     * @param con container to move
     */
    virtual Container& transfer_child(Container* con);
    /**
     * @brief Remove child from container
     * @param con container to remove
     */
    virtual Container& remove_child(Container* con);

    Container& fullscreen();
    Container& focus();

    virtual Workspace* get_workspace();

private:
    std::list<Container*> children;
};
