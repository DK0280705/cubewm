#include "workspace.h"
#include "monitor.h"

Workspace::Workspace(Workspace_id index)
    : _id(index)
    , _focused(this)
    , _monitor(nullptr)
{
    _type = CT::Workspace;
}

Workspace* Workspace::get_workspace()
{
    return this;
}

void Workspace::configure_rect(const Rectangle& rect)
{
    _rect = rect;
    configure_child_rect();
}
