#pragma once
#include "helper.h"
#include "manager.h"
#include "monitor.h"
#include "workspace.h"
#include "window.h"
#include <stdexcept>

class Connection;

class State final : public Init_once<State>
{
    Connection& _conn;
    
    Manager<Monitor>&   _mon_mgr;
    Manager<Workspace>& _wor_mgr;
    Manager<Window>&    _win_mgr;

public:
    State(Connection& conn);

    template <typename T>
    inline Manager<T>& manager()
    { throw std::runtime_error("Not existent manager"); }
    
    inline Connection& conn()
    { return _conn; }
};

template <>
inline Manager<Monitor>& State::manager()
{ return _mon_mgr; }

template <>
inline Manager<Workspace>& State::manager()
{ return _wor_mgr; }

template <>
inline Manager<Window>& State::manager()
{ return _win_mgr; }
