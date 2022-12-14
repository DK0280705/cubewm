#include "workspace.h"

Workspace::Workspace(Server* srv)
    : Container(srv)
{
    type == Container::CT_WORKSPACE;
}
