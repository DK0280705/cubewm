#pragma once
#include "../keybind.h"
#include "../helper.h"
#include <xcb/xcb_keysyms.h>
#include <xkbcommon/xkbcommon.h>

class Connection;
namespace X11 {
class Keybind : public ::Keybind
              , public Init_once<Keybind>
{  
    xkb_context* _ctx;
    xkb_keymap*  _keymap;
    xkb_state*   _state;
    xcb_key_symbols_t* _keysymbols;
public:
    Keybind();
    xkb_keycode_t keycode_from_keysym(xkb_keysym_t keysym) const override;
    virtual ~Keybind();
};
}