#pragma once
#include "container.h"

// Forward declarations
class Monitor; // #include "monitor.h"

class Workspace : public Container
{
public:
    using Workspace_id = unsigned int;

    Workspace(Workspace_id id);

    Workspace* get_workspace() override;

private:
    Workspace_id _id;
    
    friend class Monitor;
    Monitor* _monitor;
};
