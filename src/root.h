#pragma once
#include "rect.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

extern "C" {
#include <xcb/randr.h>
#include <xcb/xproto.h>
}

class Server;
class Monitor;
class Workspace;
class Container;
class Cube_window;

class Root
{
    Server& srv;

public:
    ~Root();

    static Root& instance(Server& srv); // Well, there's only one root.

    Container* focused;
    Rectangle  rect;

    void update_monitors();

    bool is_managed(xcb_window_t w);
    
    void add_monitor();
    void remove_monitor();

    Monitor* get_monitor_by(const std::string& name) const;

    void update_current_workspace();
    void update_workspace_size();
    void update_workspace_names();
    void update_workspace_viewport();
    void update_client_list();
    void update_active_window();

private:
    Root(Server& srv);

    std::unordered_set<Monitor*>                   monitors;
    std::unordered_map<uint32_t, Workspace*>       workspaces;
    std::unordered_map<xcb_window_t, Cube_window*> managed_windows;

    void update_monitors_randr();

    friend class Monitor;
};
