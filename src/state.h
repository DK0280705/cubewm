#pragma once
#include "binding.h"
#include "connection.h"
#include "helper.h"
#include "manager.h"
#include "monitor.h"
#include "workspace.h"
#include "window.h"
#include <type_traits>

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

class State final : public Init_once<State>
                  , public Observable<State>
{
public:
    enum observable : uint8_t
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
    Workspace*          _current_workspace;
    Monitor*            _current_monitor;

public:
    State(const State&)            = default;
    State(State&&)                 = delete;
    State& operator=(const State&) = delete;
    State& operator=(State&&)      = delete;

    State(Connection& conn)
        : _conn(conn)
        , _bin_mgr(Manager<Binding>::init())
        , _win_mgr(Manager<Window>::init())
        , _mon_mgr(Manager<Monitor>::init())
        , _wor_mgr(Manager<Workspace>::init())
    {}

    static State& init(Connection& conn);

    ~State() noexcept
    {
        // Recursively clear the tree.
        _mon_mgr.clear();
        // Unrelated to above.
        _bin_mgr.clear();
    }

public:
    constexpr auto conn()       const noexcept -> const Connection&
    { return _conn; }
    constexpr auto bindings()   const noexcept -> Manager<Binding>&
    { return _bin_mgr; }
    constexpr auto windows()    const noexcept -> Manager<Window>&
    { return _win_mgr; }
    constexpr auto monitors()   const noexcept -> Manager<Monitor>&
    { return _mon_mgr; }
    constexpr auto workspaces() const noexcept -> Manager<Workspace>&
    { return _wor_mgr; }

    inline void current_workspace(Workspace& workspace)
    {
        assert(_wor_mgr.contains(workspace.index()));
        _current_workspace = &workspace;
        notify<observable::current_workspace_update>();
    }

    inline void current_monitor(Monitor& monitor)
    {
        assert(_mon_mgr.contains(monitor.index()));
        _current_monitor = &monitor;
        notify<observable::current_monitor_update>();
    }

    inline auto current_workspace() const noexcept -> Workspace&
    { return *_current_workspace; }

    inline auto current_monitor()   const noexcept -> Monitor&
    { return *_current_monitor;   }

public:
    template<std::derived_from<Window> Window_t>
    inline auto manage_window(const uint32_t window_id) -> Window_t&
    {
        Window_t& window = _win_mgr.manage<Window_t>(window_id);
        move_to_workspace(window, current_workspace());
        add_window(current_workspace().window_list(), window);
        return window;
    }

    inline void unmanage_window(const uint32_t window_id)
    {
        Window& window = _win_mgr.at(window_id);
        remove_window(window.root<Workspace>().window_list(), window);
        purge_and_reconfigure(window);
        _win_mgr.unmanage(window.index());
    }

private:
    template<observable obs>
    inline constexpr const auto& _dispatch_observable_const() const noexcept
    {
        if constexpr (obs == current_workspace_update
                   || obs == current_monitor_update)
            return static_cast<const Observable<State>&>(*this);
        else if constexpr(obs == window_manager_update)
            return static_cast<const Observable<Manager<Window>>&>(_win_mgr);
        else if constexpr(obs == monitor_manager_update)
            return static_cast<const Observable<Manager<Monitor>>&>(_mon_mgr);
        else if constexpr(obs == workspace_manager_update)
            return static_cast<const Observable<Manager<Workspace>>&>(_wor_mgr);
    }

    template <observable obs>
    inline constexpr auto& _dispatch_observable() noexcept
    {
        const auto& o = _dispatch_observable_const<obs>();
        return const_cast<typename std::remove_cvref<decltype(o)>::type&>(o);
    }

public:
    template<observable obs, typename Observer>
    constexpr void connect(Observer observer) noexcept
    {
        _dispatch_observable<obs>().connect(obs, observer);
    }

    template <observable obs>
    constexpr void notify() const
    {
        _dispatch_observable_const<obs>().notify(obs);
    }

    inline void notify_all() const
    {
        Observable<State>::notify_all();
        _win_mgr.notify_all();
        _mon_mgr.notify_all();
        _wor_mgr.notify_all();
    }
};