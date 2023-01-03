#pragma once

// Forward declarations
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
struct xcb_xkb_state_notify_event_t;
struct xkb_context; // #include <xkbcommon/xkbcommon.h>
struct xkb_keymap;  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct xkb_state;   // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Server;       // #include "server.h"

// Should it be a manager class too?
class Keybind
{
public:
    static Keybind& init(Server& srv);
    void            handle(const xcb_generic_event_t& event);
    ~Keybind();

private:
    Server&            _srv;
    xkb_context*       _ctx;
    int                _device_id;
    xkb_keymap*        _keymap;
    xkb_state*         _state;
    xcb_key_symbols_t* _symbols;

    Keybind(Server& srv);
};
