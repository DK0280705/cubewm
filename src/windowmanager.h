#pragma once
#include "manager.h"
#include "window.h"
#include <list>
#include <unordered_map>

// Forward declarations
typedef unsigned int xcb_window_t;
class Server;    // #include "server.h"
class Container; // #include "container.h"
struct Window;    // #include "window.h"

// It's clear, it's a window manager class
class Window_manager : public Manager<Window>
{
public:
    using Window_id = xcb_window_t;

    Window* manage(const Window_id id)   override;
    void    unmanage(const Window_id id) override;
    
private:
    friend class Manager;
    Window_manager(Server& srv);
};

bool window_manageable(Window_manager& wm, Window_manager::Window_id id, bool must_be_mapped);
