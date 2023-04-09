#include "../connection.h"
#include "keyboard.h"
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <xcb/xcb_keysyms.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <xkbcommon/xkbcommon.h>

namespace X11 {
// We cannot get this into the X11::Keyboard class
static xcb_key_symbols_t* _keysymbols;

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
    xcb_key_symbols_free(_keysymbols);
    _keysymbols = memory::validate(xcb_key_symbols_alloc(_conn));
    _keymap = memory::validate(xkb_x11_keymap_new_from_device(_ctx, _conn, _device_id, XKB_KEYMAP_COMPILE_NO_FLAGS));
    _state  = memory::validate(xkb_x11_state_new_from_device(_keymap, _conn, _device_id));
}

Keyboard::~Keyboard()
{
    xcb_key_symbols_free(_keysymbols);
}

xcb_keycode_t keysym_to_keycode(xcb_keysym_t keysym)
{
    auto ptr = memory::c_own(xcb_key_symbols_get_keycode(_keysymbols, keysym));
    return *ptr;
}
}
