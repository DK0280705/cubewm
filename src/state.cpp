#include "state.h"
#include "manager.h"
#include "monitor.h"
#include "workspace.h"
#include "window.h"

State::State(Connection& conn)
    : _conn(conn)
    , _mon_mgr(Manager<Monitor>::init())
    , _wor_mgr(Manager<Workspace>::init())
    , _win_mgr(Manager<Window>::init())
{
};
