#pragma once
#include "binding.h"
#include "connection.h"
#include "manager.h"
#include "monitor.h"
#include "workspace.h"
#include "window.h"

#include "helper/mixins.h"

namespace X11 {
class Server;
} // namespace X11

class Timestamp
{
    static uint32_t _time;

public:
    Timestamp() = delete;
    static void update(uint32_t time) noexcept
    { _time = time; }
    static auto get() noexcept -> uint32_t
    { return _time; }
};

class State final : public helper::Init_once<State>
                  , public helper::Observable<State>
{
public:
    enum signals : uint8_t
    {
        current_workspace_update,
        current_monitor_update,
        window_manager_update,
        monitor_manager_update,
        workspace_manager_update,
    };

private:
    // The connection to wayland or x11 or whatever display i want to support
    Connection&         _conn;
    // Managers
    Manager<Binding>&   _bin_mgr;
    Manager<Window>&    _win_mgr;
    Manager<Monitor>&   _mon_mgr;
    Manager<Workspace>& _wor_mgr;
    // Current focused Workspace and Monitor and Workspace specific container
    Workspace*          _current_workspace{};
    Monitor*            _current_monitor{};

public:
    State(const State&)            = default;
    State(State&&)                 = delete;
    State& operator=(const State&) = delete;
    State& operator=(State&&)      = delete;

    explicit State(Connection& conn);

    static auto init(Connection& conn, X11::Server&) -> State&;

    ~State() noexcept;

public:
    inline constexpr auto conn()       const noexcept -> const Connection&
    { return _conn; }
    inline constexpr auto bindings()   const noexcept -> Manager<Binding>&
    { return _bin_mgr; }
    inline constexpr auto windows()    const noexcept -> Manager<Window>&
    { return _win_mgr; }
    inline constexpr auto monitors()   const noexcept -> Manager<Monitor>&
    { return _mon_mgr; }
    inline constexpr auto workspaces() const noexcept -> Manager<Workspace>&
    { return _wor_mgr; }

public:
    // Discouraged for use, use change_workspace() instead.
    void current_workspace(Workspace& workspace);
    // Discouraged for use, use change_workspace() instead.
    void current_monitor(Monitor& monitor);

    void change_workspace(Workspace& workspace) noexcept;

    /**
     * @brief Get current workspace.
     * @return Reference to current workspace
     */
    inline auto current_workspace() const noexcept -> Workspace&
    { return *_current_workspace; }

    /**
     * @brief Get current monitor.
     * @return Reference to current monitor
     */
    inline auto current_monitor()   const noexcept -> Monitor&
    { return *_current_monitor;   }

public:
    /**
     * @brief Manage a window
     * @tparam Window_t Type derived from Window class
     * @param window_id
     * @return Reference to object derived from Window class.
     */
    template<std::derived_from<Window> Window_t>
    inline auto manage_window(const uint32_t window_id) -> Window_t&
    {
        Window_t& window = _win_mgr.manage<Window_t>(window_id);
        window::move_to_workspace(window, current_workspace());
        window::add_window(current_workspace().window_list(), window);
        return window;
    }

    inline void unmanage_window(const uint32_t window_id)
    {
        Window& window = _win_mgr.at(window_id);
        window::remove_window(window.root<Workspace>().window_list(), window);
        window::purge_and_reconfigure(window);
        _win_mgr.unmanage(window.index());
    }

    inline auto create_workspace(Monitor& monitor) -> Workspace&
    {
        uint32_t id = 0;
        while (_wor_mgr.contains(id))
            ++id;
        return create_workspace(monitor, id);
    }

    inline auto create_workspace(Monitor& monitor, Manager<Workspace>::Key workspace_id) -> Workspace&
    {
        Workspace& workspace = _wor_mgr.manage(workspace_id);
        monitor.add(workspace);
        return workspace;
    }

    inline void destroy_workspace(Manager<Workspace>::Key workspace_id)
    {
        Workspace& workspace = _wor_mgr.at(workspace_id);
        assert_debug(workspace.empty(), "Workspace must be empty to safely destroy");
        assert_debug(!workspace.focused(), "Workspace must not be focused");
        Monitor& monitor = workspace.monitor();
        assert_debug(current_workspace() != workspace, "Can't destroy current workspace");
        if (monitor.current()->get() == workspace) {
            if (monitor.size() > 1) {
                monitor.current(std::ranges::prev(monitor.cend()));
            } else {
                assert(monitor != current_monitor());
            }
        }
        monitor.remove(std::ranges::find(monitor, workspace));
        _wor_mgr.unmanage(workspace.index());
    }

private:
    template<signals sig>
    static inline constexpr auto _dispatch_observable(const State& state) noexcept -> const auto&
    {
        if constexpr (sig == current_workspace_update
                   || sig == current_monitor_update)
            return static_cast<const Observable<State>&>(state);
        else if constexpr(sig == window_manager_update)
            return static_cast<const Observable<Manager<Window>>&>(state.windows());
        else if constexpr(sig == monitor_manager_update)
            return static_cast<const Observable<Manager<Monitor>>&>(state.monitors());
        else if constexpr(sig == workspace_manager_update)
            return static_cast<const Observable<Manager<Workspace>>&>(state.workspaces());
    }

    template <signals sig>
    static inline constexpr auto _dispatch_observable(State& state) noexcept -> auto&
    {
        const auto& o = State::_dispatch_observable<sig>(static_cast<const State&>(state));
        return const_cast<typename std::remove_cvref<decltype(o)>::type&>(o);
    }

public:
    template<signals sig, typename Observer>
    constexpr void connect(Observer observer) noexcept
    {
        auto& o = State::_dispatch_observable<sig>(*this);
        o.connect(sig, observer);
    }

    template <signals sig>
    constexpr void notify() const
    {
        const auto& o = State::_dispatch_observable<sig>(*this);
        o.notify(sig);
    }

    inline void notify_all() const
    {
        Observable<State>::notify_all();
        _win_mgr.notify_all();
        _mon_mgr.notify_all();
        _wor_mgr.notify_all();
    }
};