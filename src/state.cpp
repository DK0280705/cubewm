#include "state.h"
#include "keybind.h"
#include <xkbcommon/xkbcommon-keysyms.h>

uint32_t Timestamp::_time = 0;

static void _assign_default_bindings(Manager<Binding>& manager)
{
    // Mod4 means Windows key in many keyboards.
    // Mod4 + arrow
    manager.manage<Move_focus>({XKB_KEY_Left,  mod_mask::mod4}, Direction::Left);
    manager.manage<Move_focus>({XKB_KEY_Up,    mod_mask::mod4}, Direction::Up);
    manager.manage<Move_focus>({XKB_KEY_Right, mod_mask::mod4}, Direction::Right);
    manager.manage<Move_focus>({XKB_KEY_Down,  mod_mask::mod4}, Direction::Down);

    // Mod4 + Shift + arrow
    manager.manage<Move_container>({XKB_KEY_Left,  mod_mask::mod4 | mod_mask::shift}, Direction::Left);
    manager.manage<Move_container>({XKB_KEY_Up,    mod_mask::mod4 | mod_mask::shift}, Direction::Up);
    manager.manage<Move_container>({XKB_KEY_Right, mod_mask::mod4 | mod_mask::shift}, Direction::Right);
    manager.manage<Move_container>({XKB_KEY_Down,  mod_mask::mod4 | mod_mask::shift}, Direction::Down);

    // Mod4 + v
    // Mod4 + h
    // Mod4 + t
    // Mod4 + space
    manager.manage<Change_layout_type>({XKB_KEY_v, mod_mask::mod4}, Layout::Type::Vertical);
    manager.manage<Change_layout_type>({XKB_KEY_h, mod_mask::mod4}, Layout::Type::Horizontal);
}

State& State::init(Connection& conn)
{
    State& state = Init_once<State>::init(conn);
    // Set default workspace, ensure it never null.
    state._current_workspace = &state._wor_mgr.manage(0);
    // Set default monitor, at worst, atleast it handles one monitor.
    state._current_monitor = &state._mon_mgr.manage(0);

    // Add current workspace to current monitor, maintaining tree.
    state._current_monitor->add(*state._current_workspace);
    // Set rect for basic functionality. Automatically update its children.
    state._current_monitor->rect(Vector2D{{0, 0}, {640, 480}});

    // fill binding manager
    _assign_default_bindings(state._bin_mgr);

    return state;
}