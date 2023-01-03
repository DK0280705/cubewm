#include "workspacemanager.h"
#include "container.h"
#include "error.h"
#include "logger.h"
#include "server.h"
#include "window.h"
#include "workspace.h"
#include <xcb/xproto.h>

Workspace_manager::Workspace_manager(Server& srv)
    : Manager(srv), _focused_container(nullptr), _current_workspace(0)
{
}

Workspace* Workspace_manager::manage(const Workspace_id id)
{
    Workspace* ws = new Workspace(id);
    _managed.emplace(id, ws);
    return ws;
}

void Workspace_manager::unmanage(const Workspace_id id)
{
    delete _managed.at(id);
    _managed.erase(id);
}

void Workspace_manager::focus(Container* con)
{
    Workspace* ws      = con->workspace();
    _current_workspace = ws->id();
    _focused_container = con;
    ws->_focused       = con;

    xcb_window_t focus_id = con->type() == CT::Window
                                ? dynamic_cast<Window_container*>(con)->window()->id
                                : _srv.root_window();
    xcb_set_input_focus(_srv.conn(), XCB_INPUT_FOCUS_POINTER_ROOT, focus_id, _srv.conn().timestamp);
    xcb_map_window(_srv.conn(), focus_id);
    Log::debug("Set focus to {}", focus_id);
}

void Workspace_manager::place_container(const Workspace_id id, Container* con)
{
    Container* updated_con = nullptr;
    Container* focused_con = (_current_workspace == id)
                                 ? focused()
                                 : _managed.at(id)->_focused;

    switch (focused_con->type()) {
    case CT::Workspace:
        focused_con->add(con);
        updated_con = focused_con;
        break;
    case CT::Container:
        focused_con->parent()->add(con, focused_con);
        updated_con = focused_con->parent();
        break;
    default:
        assert_runtime(false, "What? you focused a dock container?");
        break;
    }

    updated_con->configure_child_rect();
    focus(con);
}

void Workspace_manager::purge_container(Container* con)
{
    // If you try to purge workspace,
    // you know if you know
    Container* parent = con->parent();
    parent->remove(con);

    Container* updated_con = parent;

    // Check if there's only one container in the parent
    // Transfer that container into the parent of the parent
    if (parent->type() == CT::Container && parent->empty()) {
        Container* grand_parent = parent->parent();
        grand_parent->transfer(parent->front())->remove(parent);
        updated_con = grand_parent;
        delete parent;
    }

    if (updated_con->size()) focus(updated_con->front());
    else focus(updated_con);
    updated_con->configure_child_rect();
}
