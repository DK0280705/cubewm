#include "../connection.h"
#include "keybind.h"
#include "x11.h"
#include <stdexcept>
#include <xcb/xcb_keysyms.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <xkbcommon/xkbcommon.h>

namespace X11 {
Keybind::Keybind()
{
    X11::Connection& conn = X11::_conn();
    _ctx = memory::validate(xkb_context_new(XKB_CONTEXT_NO_FLAGS));

    int device_id = xkb_x11_get_core_keyboard_device_id(conn);
    if (device_id == -1) throw std::runtime_error("Can't get keyboard device id");

    _keymap     = memory::validate(xkb_x11_keymap_new_from_device(_ctx, conn, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS));
    _state      = memory::validate(xkb_x11_state_new_from_device(_keymap, conn, device_id));
    _keysymbols = memory::validate(xcb_key_symbols_alloc(conn));
}

xkb_keycode_t Keybind::keycode_from_keysym(xkb_keysym_t keysym) const
{
    auto keycode = memory::c_own(xcb_key_symbols_get_keycode(_keysymbols, keysym));
    return *keycode;
}

Keybind::~Keybind()
{
    xkb_context_unref(_ctx);
    xkb_keymap_unref(_keymap);
    xkb_state_unref(_state);
    xcb_key_symbols_free(_keysymbols);
}
}
