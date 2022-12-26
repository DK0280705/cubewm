#pragma once
#include "manager.h"
#include "monitor.h"
#include <unordered_map>

// Forward declaration
class Server;    // #include "server.h"
class Monitor;   // #include "monitor.h"
class Workspace; // #include "workspace.h"

class Monitor_manager : public Manager<Monitor>
{
public:
    using Monitor_id = unsigned int;

    Monitor* manage(const Monitor_id id)   override;
    void     unmanage(const Monitor_id id) override;

    void place_workspace(const Monitor_id id, Workspace* ws);
    void purge_workspace(Workspace* ws);

    void update();

private:
    Monitor* _focused_monitor;
    
    friend class Manager;
    Monitor_manager(Server& srv);
};
