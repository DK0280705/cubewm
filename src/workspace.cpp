#include "workspace.h"
#include "monitor.h"
#include <xcb/xcb_atom.h>

Workspace::Workspace(Server& srv, Monitor& mon, uint32_t index)
    : Container(srv)
    , mon(mon)
    , index(index)
{
    type = Container::CT_WORKSPACE;
    name = std::to_string(index + 1);
    rect = mon.rect;
}

Workspace* Workspace::get_workspace()
{
    return this;
}

Workspace::~Workspace()
{
}
