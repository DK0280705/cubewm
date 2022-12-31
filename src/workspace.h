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

    inline Workspace_id id() const
    { return _id; }

    Workspace* get_workspace() override;

    void configure_rect(const Rectangle& rect);

private:
    Workspace_id _id;

    friend class Workspace_manager;
    Container* _focused;
 
    friend class Monitor;
    Monitor* _monitor;
};
