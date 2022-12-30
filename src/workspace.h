#pragma once
#include "container.h"
#include "workspacemanager.h"

// Forward declarations
class Monitor; // #include "monitor.h"

class Workspace : public Container
{
public:
    using Workspace_id = unsigned int;

    Workspace(Workspace_id id);

    Workspace* get_workspace() override;

    inline Workspace_id id() const
    { return _id; }

private:
    Workspace_id _id;

    friend class Workspace_manager;
    Container* _focused;
 
    friend class Monitor;
    Monitor* _monitor;
};
