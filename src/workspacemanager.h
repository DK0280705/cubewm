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

    void place_container(Container* con);
    void purge_container(Container* con);

private:
    Container* _focused_container;
    
    friend class Manager;
    Workspace_manager(Server& srv);
};
