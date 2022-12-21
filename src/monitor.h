#pragma once
#include "dockarea.h"
#include "workspace.h"
#include <string>
#include <vector>
#include <map>

extern "C" {
#include <xcb/randr.h>
}

class Server;
class Root;
class Workspace;
class Dockarea;
struct Rectangle;

class Monitor
{
    Server& srv;
    Root&   root;

public:
    Monitor(Server& srv, Root& root, Rectangle rect);
    ~Monitor();

    xcb_randr_output_t randr_id;

    bool primary;

    Rectangle rect;

    std::vector<std::string> names;

    void add_workspace(uint32_t id);
    void remove_workspace(uint32_t id);

    void move_to_workspace(uint32_t id);

private:
    std::list<Dockarea*>  dockareas;
    std::list<Workspace*> workspaces;
};
