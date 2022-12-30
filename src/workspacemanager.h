#pragma once
#include "manager.h"
#include "workspace.h"
#include <unordered_map>

// Forward declaration
class Server;    // #include "server.h"
class Workspace; // #include "workspace.h"
class Container; // #include "container.h"

class Workspace_manager : public Manager<Workspace>
{
public:
    using Workspace_id = unsigned int;

    Workspace* manage(const Workspace_id id)   override;
    void       unmanage(const Workspace_id id) override;

    void focus(Container* con);

    inline Container* focused() const
    { return _focused_container; }

    inline Workspace_id current() const
    { return _current_workspace; }

    void place_container(const Workspace_id id, Container* con);
    void purge_container(Container* con);

    void update_container(Container* con);

private:
    Container*   _focused_container;
    Workspace_id _current_workspace;
    
    friend class Manager;
    Workspace_manager(Server& srv);
};
