#pragma once
#include "rect.h"
#include <string>
#include <list>

/**
 * Heavily inspired from i3 window manager's data structure
 * Which implements Root -> Outputs -> Workspaces/Dockareas -> Containers
 */
class Server;
class Output;
class Workspace;
class Win;

class Container
{
protected:
    Server* srv_;

public:
    Container(Server* srv);
    virtual ~Container();

    Container* parent;

    std::list<Container*> children;

    enum
    {
        CO_HORIZONTAL,
        CO_VERTICAL
    } orientation;

    enum
    {
        CT_ROOT,
        CT_OUTPUT,
        CT_WORKSPACE,
        CT_DOCKAREA
    } type;

    Rect rect;
 
    Win* window;

    std::string name;

    Container& create_and_attach_to(Container& parent, class Win& win);
    Container& attach_to(Container& parent);
    Container& merge_to(Container& new_con);
    Container& move_to(Workspace& workspace);
    Container& move_to(Output& output);

    Container& fullscreen();
    Container& detach();
    Container& focus();

    // Must know the type first to call these functions or bruh moment.
    Output&    get_output();
    Workspace& get_workspace();
    Container& get_fullscreen();

    bool has_children();
    bool is_internal();
    bool is_managed();
    bool is_split();
    bool is_sticky();
};
