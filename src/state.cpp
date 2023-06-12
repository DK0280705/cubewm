#include "state.h"
#include "keybind.h"
#include "x11/monitor.h"
#include "x11/window.h"
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

auto State::init(Connection& conn, X11::Server&) -> State&
{
    State& state = Init_once<State>::init(conn);

    // First, load all monitors.
    X11::monitor::load_all(state);
    // Set current monitor to the first monitor.
    state.current_monitor(state.monitors().at(0));
    // Set default workspace.
    state.current_workspace(state.workspaces().manage(0));
    // Set current monitor children to the default workspace
    state.current_monitor().add(state.current_workspace());
    // Update rect to update workspace rect.
    state.current_monitor().update_rect();

    // Second, load all windows.
    X11::window::load_all(state);
    // Focus the last window in the current workspace.
    window::focus_last(state.current_workspace().window_list());

    // Third, fill the binding manager
    _assign_default_bindings(state._bin_mgr);

    return state;
}