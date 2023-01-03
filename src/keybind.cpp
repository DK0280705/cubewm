#include "keybind.h"
#include "error.h"
#include "logger.h"
#include "server.h"
#include <xcb/xproto.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <xkbcommon/xkbcommon.h>
#define explicit _explicit
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xkb.h>
#undef explicit

Keybind::Keybind(Server& srv)
    : _srv(srv)
    , _ctx(check_null(xkb_context_new(XKB_CONTEXT_NO_FLAGS),
                      "Can't initialize XKB"))
    , _device_id(check_return(xkb_x11_get_core_keyboard_device_id(_srv.conn()),
                              -1,
                              "Can't get keyboard device id"))
    , _keymap(check_null(
          xkb_x11_keymap_new_from_device(
              _ctx, _srv.conn(), _device_id, XKB_KEYMAP_COMPILE_NO_FLAGS),
          "Can't initialize keymap"))
    , _state(check_null(xkb_x11_state_new_from_device(_keymap,
                                                      _srv.conn(),
                                                      _device_id),
                        "Can't initialize keyboard state"))
    , _symbols(xcb_key_symbols_alloc(_srv.conn()))
{
}

Keybind& Keybind::init(Server& srv)
{
    static Keybind kb(srv);
    assert_init(kb);
    return kb;
}

void Keybind::handle(const xcb_generic_event_t& event)
{
    switch (event.pad0) {
    case XCB_XKB_STATE_NOTIFY:
    {
        const auto& ev = *(xcb_xkb_state_notify_event_t*)(&event);
        xkb_state_update_mask(_state,
                              ev.baseMods,
                              ev.latchedMods,
                              ev.lockedMods,
                              ev.baseGroup,
                              ev.latchedGroup,
                              ev.lockedGroup);
        Log::debug("Updated state");
        xcb_ungrab_key(_srv.conn(),
                       XCB_GRAB_ANY,
                       _srv.root_window(),
                       XCB_BUTTON_MASK_ANY);
        // Just for test
        xcb_keycode_t* code = xcb_key_symbols_get_keycode(_symbols, XKB_KEY_q);
        xcb_grab_key(_srv.conn(),
                     0,
                     _srv.root_window(),
                     XCB_MOD_MASK_4,
                     *code,
                     XCB_GRAB_MODE_SYNC,
                     XCB_GRAB_MODE_ASYNC);
        free(code);
        break;
    }
    case XCB_XKB_MAP_NOTIFY:
        Log::debug("XKB Map Notify");
        break;
    case XCB_XKB_NEW_KEYBOARD_NOTIFY:
        Log::debug("XKB New Keyboard Notify");
        break;
    }
}

Keybind::~Keybind()
{
    xkb_state_unref(_state);
    xkb_keymap_unref(_keymap);
    xkb_context_unref(_ctx);
}
