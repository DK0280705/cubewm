#include "monitor.h"
#include "server.h"
#include "root.h"
#include "workspace.h"
#include <algorithm>

Monitor::Monitor(Server& srv, Root& root, Rectangle rect)
    : srv(srv)
    , root(root)
    , rect(rect)
{
    // Add default workspace
    add_workspace(0);
}

void Monitor::add_workspace(uint32_t id)
{
    Workspace* ws = new Workspace(srv, *this, id);
    if (workspaces.empty()) {
        workspaces.push_back(ws);
    } else {
        workspaces.insert(std::next(workspaces.begin(), id), ws);
    }
    root.workspaces.emplace(id, ws);

    root.update_workspace_size();
    root.update_workspace_names();
    root.update_workspace_viewport();
}

void Monitor::remove_workspace(uint32_t id)
{
    Workspace* ws = root.workspaces.at(id);
    workspaces.erase(std::find(workspaces.begin(), workspaces.end(), ws));
    root.workspaces.erase(id);
    delete ws;

    root.update_workspace_size();
    root.update_workspace_names();
    root.update_workspace_viewport();
}

Monitor::~Monitor()
{
    // Cleanup
    std::for_each(workspaces.begin(), workspaces.end(), [](const auto& ws) { delete ws; });
    std::for_each(dockareas.begin(), dockareas.end(), [](const auto& da) { delete da; });
}
