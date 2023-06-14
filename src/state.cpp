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

State::State(Connection &conn)
    : _conn(conn)
    , _bin_mgr(Manager<Binding>::init())
    , _win_mgr(Manager<Window>::init())
    , _mon_mgr(Manager<Monitor>::init())
    , _wor_mgr(Manager<Workspace>::init())
{}

auto State::init(Connection& conn, X11::Server&) -> State&
{
    State& state = Init_once<State>::init(conn);

    // First, load all monitors.
    X11::monitor::load_all(state);
    // Set current monitor to the first monitor.
    state._current_monitor = &state.monitors().at(0);
    // Set default workspace.
    state._current_workspace = &state.workspaces().manage(0);
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

void State::current_workspace(Workspace &workspace)
{
    assert(_wor_mgr.contains(workspace.index()));
    _current_workspace = &workspace;
}

void State::current_monitor(Monitor &monitor)
{
    assert(_mon_mgr.contains(monitor.index()));
    _current_monitor = &monitor;
}

void State::change_workspace(Workspace& workspace) noexcept
{
    assert(current_workspace() != workspace);
    if (workspace.monitor() != current_monitor()) {
        current_monitor().unfocus();
        current_monitor(workspace.monitor());
        if (current_monitor().current()->get() != workspace)
            current_monitor().current(std::ranges::find(current_monitor(), workspace));
        current_monitor().focus();
        notify<signals::current_monitor_update>();
    } else {
        current_workspace().unfocus();
        workspace.monitor().current(std::ranges::find(workspace.monitor(), workspace));
        workspace.focus();
    }
    current_workspace(workspace);
    notify<signals::current_workspace_update>();
}

State::~State() noexcept
{
    // Recursively clear the tree.
    _mon_mgr.clear();
    // Unrelated to above.
    _bin_mgr.clear();
}
