#include "keyboard.h"
#include "binding.h"
#include "helper.h"
#include <xcb/xproto.h>
#include <xkbcommon/xkbcommon-keysyms.h>

static void _assign_default_bindings(Manager<Binding, Keybind>& manager)
{
    // Mod4 means Windows key in many keyboards.
    // Mod4 + arrow
    manager.manage<Move_focus>({XKB_KEY_Left,  mod_mask::mod4}, Direction::Left);
    manager.manage<Move_focus>({XKB_KEY_Up,    mod_mask::mod4}, Direction::Up);
    manager.manage<Move_focus>({XKB_KEY_Right, mod_mask::mod4}, Direction::Right);
    manager.manage<Move_focus>({XKB_KEY_Down,  mod_mask::mod4}, Direction::Down);

    // Mod4 + v
    // Mod4 + h
    // Mod4 + t
    // Mod4 + space
    manager.manage<Change_layout_type>({XKB_KEY_v, mod_mask::mod4}, Layout::Type::Vertical);
    manager.manage<Change_layout_type>({XKB_KEY_h, mod_mask::mod4}, Layout::Type::Horizontal);
}

Keyboard::Keyboard(const Connection& conn)
    : _conn(conn)
    , _ctx(memory::validate(xkb_context_new(XKB_CONTEXT_NO_FLAGS)))
    , _keymap(nullptr)
    , _state(nullptr)
{
    _assign_default_bindings(_binding_manager);
}

void Keyboard::_clear_keymap()
{
    xkb_state_unref(_state);
    xkb_keymap_unref(_keymap);
}

Keyboard::~Keyboard()
{
    _binding_manager.clear();
    _clear_keymap();
    xkb_context_unref(_ctx);
}