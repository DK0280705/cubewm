#include "monitor.h"
#include "workspace.h"
#include <algorithm>

Monitor::Monitor(const Monitor_id id) noexcept
    : _id(id)
    , _rect({0, 0, 0, 0})
    , _primary(false)
{
}

void Monitor::add_workspace(Workspace* ws)
{
    ws->_monitor = this;
    _workspaces.push_back(ws);
}

void Monitor::transfer_workspace(Workspace* ws)
{
    ws->_monitor->remove_workspace(ws);
    add_workspace(ws);
}

void Monitor::remove_workspace(Workspace* ws)
{
    ws->_monitor = nullptr;
    _workspaces.remove(ws);
}

Monitor::~Monitor()
{}
