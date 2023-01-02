#pragma once

// Forward declarations
struct xkb_context; // #include <xkbcommon/xkbcommon.h>
struct xkb_keymap;  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct xkb_state;   // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Server;       // #include "server.h"

// Should it be a manager class too?
class Keybind
{
public:
    static Keybind& init(Server& srv);
    ~Keybind();
private:
    Server&      _srv;
    xkb_context* _ctx;
    int          _device_id;
    xkb_keymap*  _keymap;
    xkb_state*   _state;

    Keybind(Server& srv);
};
