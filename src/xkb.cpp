#include "xkb.h"
#include "binding.h"
#include "helper.h"
#include <xcb/xproto.h>
#include <xkbcommon/xkbcommon.h>

XKB::XKB(const Connection& conn)
    : _conn(conn)
    , _ctx(memory::validate(xkb_context_new(XKB_CONTEXT_NO_FLAGS)))
    , _keymap(nullptr)
    , _state(nullptr)
{
}

void XKB::_clear_keymap()
{
    xkb_state_unref(_state);
    xkb_keymap_unref(_keymap);
}

XKB::~XKB()
{
    _clear_keymap();
    xkb_context_unref(_ctx);
}

auto create_keybind(XKB& xkb, uint32_t keycode, uint16_t modifiers) noexcept -> Keybind
{
    xkb_keysym_t keysym = xkb_state_key_get_one_sym(xkb.state(), keycode);
    return {keysym, static_cast<uint16_t>(modifiers & ~XCB_MOD_MASK_LOCK)};
}