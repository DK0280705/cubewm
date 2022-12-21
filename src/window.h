#pragma once
#include <string>

// Forward Declarations
typedef uint32_t xcb_window_t;
class Server;
class Container;

class Cube_window
{
    Server* srv;

public:
    Cube_window(Server* srv);
    ~Cube_window();

    xcb_window_t id;

    Container* container;

    std::string name;
    std::string role;
    bool fullscreen;
    bool hidden;
};
