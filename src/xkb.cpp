#include "xkb.h"
#include "error.h"
#include "helper/memory.h"
#include <xcb/xproto.h>
#include <xkbcommon/xkbcommon.h>

XKB::XKB(Connection& conn)
    : _conn(conn)
    , _ctx(memory::validate(xkb_context_new(XKB_CONTEXT_NO_FLAGS)))
    , _keymap(nullptr)
    , _state(nullptr)
{
    _instance = this;
}

void XKB::_clear_keymap()
{
    xkb_state_unref(_state);
    xkb_keymap_unref(_keymap);
}

auto XKB::instance() noexcept -> XKB&
{
    assert(_instance);
    return *_instance;
}

auto XKB::create_keybind(xkb_keycode_t keycode, xkb_mod_mask_t modifiers) noexcept -> Keybind
{
    xkb_keysym_t keysym = xkb_state_key_get_one_sym(XKB::instance().state(), keycode);
    return {keysym, static_cast<uint16_t>(modifiers & ~XCB_MOD_MASK_LOCK)};
}

XKB::~XKB()
{
    _clear_keymap();
    xkb_context_unref(_ctx);
}
