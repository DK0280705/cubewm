#pragma once
#include "connection.h"
#include "server.h"
#include "keyboard.h"
#include "helper.h"
#include "manager.h"
#include "monitor.h"
#include "workspace.h"
#include "window.h"
#include <stdexcept>
#include <type_traits>

class Timestamp
{
    unsigned int _time = 0;

public:
    operator unsigned int() const
    { return _time; }

    void update(unsigned int time)
    { _time = time; }
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
    Manager<Window>&    _win_mgr;
    Manager<Monitor>&   _mon_mgr;
    Manager<Workspace>& _wor_mgr;
    // The server manager, late initialization.
    Server*             _server{};
    // The keyboard manager, late initialization.
    Keyboard*           _keyboard{};
    // Current focused Workspace and Monitor and Workspace specific container
    Workspace*          _current_workspace;
    Monitor*            _current_monitor;

    static Timestamp    _timestamp;

public:
    State(const State&)            = default;
    State(State&&)                 = delete;
    State& operator=(const State&) = delete;
    State& operator=(State&&)      = delete;

    State(Connection& conn)
        : _conn(conn)
        , _win_mgr(Manager<Window>::init())
        , _mon_mgr(Manager<Monitor>::init())
        , _wor_mgr(Manager<Workspace>::init())
        // Set default workspace, ensure it never null.
        , _current_workspace(&_wor_mgr.manage(0))
        // Set default monitor, at worst, atleast it handles one monitor.
        , _current_monitor(&_mon_mgr.manage(0))
    {
        // Connect signals to managers.
        _win_mgr.connect([&](const auto&) { this->notify(State::window_manager_update); });
        _mon_mgr.connect([&](const auto&) { this->notify(State::monitor_manager_update); });
        _wor_mgr.connect([&](const auto&) { this->notify(State::workspace_manager_update); });

        // Add current workspace to current monitor, maintaining tree.
        _current_monitor->add(*_current_workspace);
        // Set rect for basic functionality.
        _current_monitor->rect(Vector2D{{0, 0}, {640, 480}});
    }

    ~State() noexcept
    {
        // Recursively clear the tree.
        _mon_mgr.clear();
    }

public:
    // Late initialization, can't figure out a better way to do.
    template <std::derived_from<Keyboard> T>
    constexpr void init_keyboard(auto&&... args)
    {
        _keyboard = &T::init(std::forward<decltype(args)>(args)...);
    }

    template <std::derived_from<Server> T>
    constexpr void init_server(auto&&... args)
    {
        _server = &T::init(std::forward<decltype(args)>(args)...);
    }

public:
    constexpr const Connection& conn() const noexcept
    { return _conn; }

    template <std::derived_from<Managed<unsigned int>> T>
    constexpr Manager<T>& manager() const noexcept
    {
        // Forbids all kind of foreign type
        // This kind of template is fancy
        throw std::exception();
    }

    constexpr Keyboard& keyboard() const noexcept
    {
        // Simply boom if there's no instance;
        assert(_keyboard);
        return *_keyboard;
    }

    constexpr Server& server() const noexcept
    {
        assert(_server);
        return *_server;
    }

    inline void current_workspace(Workspace& ws)
    {
        _current_workspace = &ws;
        notify(observable::current_workspace_update);
    }

    inline Workspace& current_workspace() const noexcept
    { return *_current_workspace; }

    inline void current_monitor(Monitor& mon)
    {
        _current_monitor = &mon;
        notify(observable::current_monitor_update);
    }

    inline Monitor& current_monitor() const noexcept
    { return *_current_monitor; }


    static inline Timestamp& timestamp() noexcept
    { return _timestamp; }
};

template <>
constexpr Manager<Window>&    State::manager() const noexcept
{ return _win_mgr; }

template <>
constexpr Manager<Monitor>&   State::manager() const noexcept
{ return _mon_mgr; }

template <>
constexpr Manager<Workspace>& State::manager() const noexcept
{ return _wor_mgr; }