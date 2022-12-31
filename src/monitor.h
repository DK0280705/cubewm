#pragma once
#include "rect.h"
#include <list>

class Workspace; // #include "workspace.h"

class Monitor
{
public:
    using Monitor_id          = unsigned int;
    using Workspace_container = std::list<Workspace*>;

    Monitor(const Monitor_id id) noexcept;

    void add_workspace(Workspace* ws);
    void transfer_workspace(Workspace* ws);
    void remove_workspace(Workspace* ws);

    ~Monitor();

private:
    friend class Monitor_manager;

    Monitor_id _id;
    Rectangle  _rect;
    bool       _primary;

    // For structural design
    // Removing an object from a container does not delete the object's memory.
    Workspace_container _workspaces;
};
