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
    /**
     * Switch to desired workspace.
     * Automatically handles workspace in different monitor.
     * @param workspace
     */
    void switch_workspace(Workspace& workspace) noexcept;

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
     * @param window_id
     * @param type Display type for window
     * @param workspace Workspace to contain window
     * @return Reference to object derived from Window class.
     */
    auto manage_window(Manager<Window>::Key window_id, Window::Display_type type) -> Window&;

    void unmanage_window(Manager<Window>::Key window_id);

    auto create_workspace(Monitor& monitor) -> Workspace&;

    auto create_workspace(Monitor& monitor, Manager<Workspace>::Key workspace_id) -> Workspace&;

    auto get_or_create_workspace(Manager<Workspace>::Key workspace_id) -> Workspace&;

    void destroy_workspace(Manager<Workspace>::Key workspace_id);

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
    template<signals sig, typename Func>
    constexpr void connect(Func&& func) noexcept
    {
        auto& o = State::_dispatch_observable<sig>(*this);
        o.connect(sig, std::forward<Func>(func));
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
