#include "keyboard.h"
#include "helper.h"

Keyboard::Keyboard(const Connection& conn)
    : _conn(conn)
    , _ctx(memory::validate(xkb_context_new(XKB_CONTEXT_NO_FLAGS)))
{
}

void Keyboard::_clear_keymap()
{
    xkb_state_unref(_state);
    xkb_keymap_unref(_keymap);
}

Keyboard::~Keyboard()
{
    xkb_context_unref(_ctx);
    _clear_keymap();
}