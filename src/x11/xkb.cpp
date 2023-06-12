#include "xkb.h"

#include "../connection.h"
#include "../helper/memory.h"
#include <xkbcommon/xkbcommon-x11.h>

namespace X11 {

XKB::XKB(::Connection& conn)
    : ::XKB(conn)
    , _device_id(xkb_x11_get_core_keyboard_device_id(conn)) //goofy idea
{
    assert_runtime<std::runtime_error>(_device_id != -1, "Can't get keyboard device id");
    update_keymap();
}

void XKB::update_keymap()
{
    _clear_keymap();
    _keymap = memory::validate(xkb_x11_keymap_new_from_device(_ctx, _conn, _device_id, XKB_KEYMAP_COMPILE_NO_FLAGS));
    _state  = memory::validate(xkb_x11_state_new_from_device(_keymap, _conn, _device_id));
}

XKB::~XKB() = default;
}
