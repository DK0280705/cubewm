#include "monitormanager.h"
#include "monitor.h"
#include "server.h"
#include "workspace.h"

Monitor_manager::Monitor_manager(Server& srv)
    : Manager(srv)
    , _focused_monitor(nullptr)
{
}

Monitor* Monitor_manager::manage(const Monitor_id id)
{
    Monitor* mon = new Monitor(id);
    _managed.emplace(id, mon);
    return mon;
}

void Monitor_manager::unmanage(const Monitor_id id)
{
    delete _managed.at(id);
    _managed.erase(id);
}

void Monitor_manager::place_workspace(const Monitor_id id, Workspace* ws)
{
    assert_runtime(_managed.contains(id), "Invalid monitor id");
    _managed.at(id)->add_workspace(ws);
}

void Monitor_manager::purge_workspace(Workspace* ws)
{
    _focused_monitor->remove_workspace(ws);
}

void Monitor_manager::update()
{
    // For now, let's just add one monitor;
    if (_managed.empty()) {
        Monitor* mon = manage(0);
        mon->rect    = _srv.default_rect();
        mon->name    = "xroot-0";
        mon->primary = true;
    }
}
