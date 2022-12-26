#include "workspace.h"
#include "monitor.h"
#include <xcb/xcb_atom.h>

Workspace::Workspace(Workspace_id index)
    : _id(index)
{
    type = Container::CT_WORKSPACE;
    name = std::to_string(index + 1);
}

Workspace* Workspace::get_workspace()
{
    return this;
}
