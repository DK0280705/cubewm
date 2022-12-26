#pragma once
#include "rect.h"
#include <list>
#include <string>

class Workspace; // #include "workspace.h"

class Monitor
{
public:
    using Monitor_id          = unsigned int;
    using Monitor_name        = std::string;
    using Workspace_container = std::list<Workspace*>;

    Monitor_id   index;
    Monitor_name name;
    Rectangle    rect;
    bool         primary;

    Monitor(const Monitor_id index);

    void add_workspace(Workspace* ws);
    void transfer_workspace(Workspace* ws);
    void remove_workspace(Workspace* ws);

    ~Monitor();

private:
    // For structural design
    // Removing an object from a container does not delete the object's memory.
    Workspace_container _workspaces;
};
