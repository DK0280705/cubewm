#include "keybind.h"
#include "error.h"
#include "server.h"
#include <xkbcommon/xkbcommon-x11.h>
#include <xkbcommon/xkbcommon.h>

Keybind::Keybind(Server& srv)
    : _srv(srv)
    , _ctx(xkb_context_new(XKB_CONTEXT_NO_FLAGS))
    , _device_id(xkb_x11_get_core_keyboard_device_id(_srv.conn()))
    , _keymap(xkb_x11_keymap_new_from_device(check_null(_ctx,
                                                        "Can't initialize XKB"),
                                             _srv.conn(),
                                             check_return(_device_id,
                                                          _device_id != -1,
                                                          "Can't get keyboard "
                                                          "device id"),
                                             XKB_KEYMAP_COMPILE_NO_FLAGS))
    , _state(
          xkb_x11_state_new_from_device(check_null(_keymap,
                                                   "Can't initialize keymap"),
                                        _srv.conn(),
                                        _device_id))
{
}

Keybind& Keybind::init(Server& srv)
{
    static Keybind kb(srv);
    assert_init(kb);
    return kb;
}

Keybind::~Keybind()
{
    xkb_context_unref(_ctx);
}
