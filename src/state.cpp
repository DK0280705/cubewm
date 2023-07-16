#include "state.h"
#include "keybind.h"

#include "x11/monitor.h"
#include "x11/window.h"
#include "x11/ewmh.h"

#include <xkbcommon/xkbcommon-keysyms.h>

uint32_t Timestamp::_time = 0;

static void _assign_default_bindings(Manager<Binding>& manager)
{
    using namespace binding;
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
    manager.manage<Change_layout_type>({XKB_KEY_v, mod_mask::mod4}, Layout::Containment_type::Vertical);
    manager.manage<Change_layout_type>({XKB_KEY_h, mod_mask::mod4}, Layout::Containment_type::Horizontal);

    // Mod4 + 1
    // ...
    manager.manage<Switch_workspace>({XKB_KEY_1, mod_mask::mod4}, 0);
    manager.manage<Switch_workspace>({XKB_KEY_2, mod_mask::mod4}, 1);
    manager.manage<Switch_workspace>({XKB_KEY_3, mod_mask::mod4}, 2);
    manager.manage<Switch_workspace>({XKB_KEY_4, mod_mask::mod4}, 3);
    manager.manage<Switch_workspace>({XKB_KEY_5, mod_mask::mod4}, 4);
    manager.manage<Switch_workspace>({XKB_KEY_6, mod_mask::mod4}, 5);
    manager.manage<Switch_workspace>({XKB_KEY_7, mod_mask::mod4}, 6);
    manager.manage<Switch_workspace>({XKB_KEY_8, mod_mask::mod4}, 7);
    manager.manage<Switch_workspace>({XKB_KEY_9, mod_mask::mod4}, 8);
    manager.manage<Switch_workspace>({XKB_KEY_0, mod_mask::mod4}, 9);

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
    state._current_workspace = &state.create_workspace(state.current_monitor());

    // Focus current monitor and workspace.
    auto it = std::ranges::find(state.current_monitor(), state.current_workspace());
    state.current_monitor().current(it);
    state.current_monitor().focus();
    // Update monitor rect to update workspace rect.
    state.current_monitor().update_rect();

    // Second, load all windows.
    X11::window::load_all(state);
    // Focus the last window in the current workspace.
    if (state.current_workspace().has_window())
        state.current_workspace().current_window().focus();

    // Third, fill the binding manager
    _assign_default_bindings(state._bin_mgr);

    // Register emwh functions
    state.connect<State::current_workspace_update>([](const State& state) {
        // Updating current workspace means updating current active window.
        ewmh::update_net_current_desktop(state.current_workspace());
        uint32_t window_id = state.current_workspace().has_window()
                           ? state.current_workspace().current_window().index()
                           : XCB_NONE;
        ewmh::update_net_active_window(window_id);
    });
    state.connect<State::window_manager_update>(ewmh::update_net_client_list);
    state.connect<State::workspace_manager_update>(ewmh::update_net_desktop_names);
    state.connect<State::workspace_manager_update>(ewmh::update_net_number_of_desktops);
    state.notify_all();

    return state;
}

void State::switch_workspace(Workspace& workspace) noexcept
{
    // Current workspace must not be the same as specified workspace
    assert(current_workspace() != workspace);
    auto& last_workspace = current_workspace();

    if (workspace.monitor() != current_monitor()) {
        // Set current monitor to workspace's monitor.
        current_monitor().unfocus();
        _current_monitor = &workspace.monitor();
        // Set workspace as current workspace on monitor.
        if (current_monitor().current() != workspace)
            current_monitor().current(std::ranges::find(current_monitor(), workspace));
        current_monitor().focus();
        notify<signals::current_monitor_update>();
    } else {
        last_workspace.unfocus();
        workspace.monitor().current(std::ranges::find(workspace.monitor(), workspace));
        workspace.focus();
    }

    _current_workspace = &workspace;
    notify<signals::current_workspace_update>();

    // Purge last workspace if empty.
    if (last_workspace.size() == 1 && last_workspace.floating_layout().empty())
        destroy_workspace(last_workspace.index());
}

auto State::manage_window(const uint32_t window_id, Window::Display_type type) -> Window&
{
    Window& window = _win_mgr.manage(window_id, type);
    window::move_to_workspace(window, current_workspace());
    window.focus();
    return window;
}

void State::unmanage_window(const uint32_t window_id)
{
    Window& window = _win_mgr.at(window_id);
    window.root<Workspace>().remove_window(window);
    window::purge_and_reconfigure(window);
    _win_mgr.unmanage(window.index());
}

auto State::create_workspace(Monitor& monitor) -> Workspace&
{
    uint32_t id = 0;
    while (_wor_mgr.contains(id))
        ++id;
    return create_workspace(monitor, id);
}

auto State::create_workspace(Monitor& monitor, Manager<Workspace>::Key workspace_id) -> Workspace&
{
    Workspace& workspace = _wor_mgr.manage(workspace_id);
    monitor.add_child(workspace);
    monitor.update_rect();
    return workspace;
}

auto State::get_or_create_workspace(Manager<Workspace>::Key workspace_id) -> Workspace&
{
    return workspaces().contains(workspace_id)
        ? workspaces().at(workspace_id)
        : create_workspace(current_monitor(), workspace_id);
}

void State::destroy_workspace(Manager<Workspace>::Key workspace_id)
{
    Workspace& workspace = _wor_mgr.at(workspace_id);
    assert_debug(workspace.size() == 1 && workspace.floating_layout().empty(),
                 "Workspace must be empty to safely destroy");
    assert_debug(!workspace.focused(), "Workspace must not be focused");
    Monitor& monitor = workspace.monitor();
    assert_debug(current_workspace() != workspace, "Can't destroy current workspace");
    if (monitor.current() == workspace) {
        if (monitor.size() > 1) {
            monitor.current(std::ranges::prev(monitor.cend()));
        } else {
            assert(monitor != current_monitor());
        }
    }
    monitor.remove_child(std::ranges::find(monitor, workspace));
    _wor_mgr.unmanage(workspace.index());
}

State::~State() noexcept
{
    // Recursively clear the tree.
    _mon_mgr.clear();
    // Unrelated to above.
    _bin_mgr.clear();
}


