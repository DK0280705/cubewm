#pragma once
#include "server.h"
#include "keybind.h"
#include "helper.h"
#include "manager.h"
#include "monitor.h"
#include "workspace.h"
#include "window.h"
#include <stdexcept>
#include <type_traits>

class Connection;
class Server;

class State final : public Init_once<State>
{
    Connection& _conn;

    Manager<Monitor>&   _mon_mgr;
    Manager<Workspace>& _wor_mgr;
    Manager<Window>&    _win_mgr;

    Server*  _server;
    Keybind* _keybind;

    Workspace* _current_workspace;

public:
    State(Connection& conn)
        : _conn(conn)
        , _mon_mgr(Manager<Monitor>::init())
        , _wor_mgr(Manager<Workspace>::init())
        , _win_mgr(Manager<Window>::init())
    {}

    ~State()
    {
        // Recursively clear the tree.
        for (const auto& mon : _mon_mgr) {
            delete mon.second;
        }
    }

public:
    inline Connection& conn() noexcept
    { return _conn; }

    template <derived_from<Managed> T>
    inline Manager<T>& manager() noexcept
    { assert(false); }

    template <derived_from<Keybind> T>
    inline void init_keybind()
    { _keybind = &T::init(); }

    inline Keybind& keybind() noexcept
    // Simply boom if there's no instance;
    { assert(_keybind); return *_keybind; }

    template <derived_from<Server> T>
    inline void init_server()
    { _server = &T::init(*this); }

    inline Server& server() noexcept
    { assert(_server); return *_server; }

    inline void current_workspace(Workspace* ws) noexcept
    { _current_workspace = ws; }

    inline Workspace* current_workspace() const noexcept
    { return _current_workspace; }
};

template <>
inline Manager<Monitor>& State::manager() noexcept
{ return _mon_mgr; }

template <>
inline Manager<Workspace>& State::manager() noexcept
{ return _wor_mgr; }

template <>
inline Manager<Window>& State::manager() noexcept
{ return _win_mgr; }
