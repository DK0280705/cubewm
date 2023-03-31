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

class State final : public Init_once<State>
{
    Connection& _conn;

    Manager<Monitor>&   _mon_mgr;
    Manager<Workspace>& _wor_mgr;
    Manager<Window>&    _win_mgr;

    Server*  _server;
    Keyboard* _keyboard;

    Workspace* _current_workspace;
    Monitor*   _current_monitor;

public:
    static uint32_t timestamp; // It just a timestamp duh...

    State(Connection& conn)
        : _conn(conn)
        , _mon_mgr(Manager<Monitor>::init())
        , _wor_mgr(Manager<Workspace>::init())
        , _win_mgr(Manager<Window>::init())
    {
        // Set default workspace, ensure it never null.
        _current_workspace = _wor_mgr.manage(0);
        // Set default monitor, at worst, atleast it handles one monitor.
        _current_monitor = _mon_mgr.manage(0);
        _current_monitor->add(_current_workspace);
        // Set rect for basic functionality.
        _current_monitor->rect(Vector2D{{0, 0}, {640, 480}});
    }

    ~State()
    {
        // Recursively clear the tree.
        for (const auto& mon : _mon_mgr) {
            delete mon.second;
        }
    }

public:
    constexpr const Connection& conn() const noexcept
    { return _conn; }

    template <derived_from<Managed> T>
    constexpr Manager<T>& manager() noexcept
    { throw std::exception(); }

    template <derived_from<Keyboard> T>
    constexpr void init_keyboard(auto&&... args)
    { _keyboard = &T::init(std::forward<decltype(args)>(args)...); }

    constexpr Keyboard& keyboard() noexcept
    // Simply boom if there's no instance;
    { assert(_keyboard); return *_keyboard; }

    template <derived_from<Server> T>
    constexpr void init_server(auto&&... args)
    { _server = &T::init(std::forward<decltype(args)>(args)...); }

    constexpr Server& server() noexcept
    { assert(_server); return *_server; }

    inline void current_workspace(Workspace* ws) noexcept
    { _current_workspace = ws; }

    constexpr Workspace* current_workspace() const noexcept
    { return _current_workspace; }
};

template <>
constexpr Manager<Monitor>& State::manager() noexcept
{ return _mon_mgr; }

template <>
constexpr Manager<Workspace>& State::manager() noexcept
{ return _wor_mgr; }

template <>
constexpr Manager<Window>& State::manager() noexcept
{ return _win_mgr; }
