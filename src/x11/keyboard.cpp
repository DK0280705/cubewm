#include "../connection.h"
#include "keyboard.h"
#include <stdexcept>
#include <unordered_map>
#include <xcb/xcb_keysyms.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <xkbcommon/xkbcommon.h>

namespace X11 {

Keyboard::Keyboard(const ::Connection& conn)
    : ::Keyboard(conn)
    , _device_id(xkb_x11_get_core_keyboard_device_id(conn)) //goofy idea
{
    if (_device_id == -1)
        throw std::runtime_error("Can't get keyboard device id");
    update_keymap();
}

void Keyboard::update_keymap()
{
    _clear_keymap();
    _keymap = memory::validate(xkb_x11_keymap_new_from_device(_ctx, _conn, _device_id, XKB_KEYMAP_COMPILE_NO_FLAGS));
    _state  = memory::validate(xkb_x11_state_new_from_device(_keymap, _conn, _device_id));
}

Keyboard::~Keyboard()
{
}
}
