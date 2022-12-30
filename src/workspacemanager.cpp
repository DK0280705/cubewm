#include "workspacemanager.h"
#include "container.h"
#include "error.h"
#include "server.h"
#include "window.h"
#include "workspace.h"
#include "xwrap.h"
#include <cmath>
#include <xcb/xproto.h>

Workspace_manager::Workspace_manager(Server& srv)
    : Manager(srv)
    , _focused_container(nullptr)
    , _current_workspace(0)
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
    Workspace* ws      = con->get_workspace();
    _current_workspace = ws->id();
    _focused_container = con;
    ws->_focused       = con;
}

void Workspace_manager::place_container(const Workspace_id id, Container* con)
{
    Container* updated_con = nullptr;
    Container* focused_con = (_current_workspace == id) ? focused() : _managed.at(id)->_focused;
    
    switch (focused_con->type) {
    case Container::CT_WORKSPACE:
        focused_con->add_child(con);
        updated_con = focused_con;
        break;
    case Container::CT_CONTAINER:
        if (focused_con->orientation == focused_con->parent()->orientation) {
            focused_con->parent()->add_child(con, focused_con);
            updated_con = focused_con->parent();
        } else {
            Container* split_con = new Container();
            Container* parent    = focused_con->parent();
            split_con->transfer_child(focused())
                     ->add_child(con);
            parent->add_child(split_con);
            updated_con = parent;
        }
        break;
    default:
        assert_runtime(false, "What? you focused a dock container?");
        break;
    }
    
    update_container(updated_con);
    focus(con);
}

void Workspace_manager::purge_container(Container* con)
{
    // If you try to purge workspace,
    // you know if you know
    Container* parent = con->parent();
    parent->remove_child(con);

    Container* updated_con = parent;
    
    // Check if there's only one container in the parent
    // Transfer that container into the parent of the parent
    if (parent->type == Container::CT_CONTAINER && parent->size() == 1) {
        Container* grand_parent = parent->parent();
        grand_parent->transfer_child(parent->front())
                    ->remove_child(parent);
        updated_con = grand_parent;
        delete parent;
    }

    if (updated_con->size()) {
        focus(updated_con->front());
    } else {
        focus(updated_con);
    }
    update_container(updated_con);
}

static void update_container_recurse(Container* parent)
{
    const Rectangle parent_rect = parent->rect;

    uint32_t next_pos_x = parent->rect.x;
    uint32_t next_pos_y = parent->rect.y;
    for (const auto& child : *parent) {
        // All of the configured resize will be reset
        const float mult = 1.0f / static_cast<float>(parent->size());
        if (parent->orientation == Container::CO_HORIZONTAL) {
            child->rect.height = parent_rect.height;
            child->rect.width  = std::round(mult * parent_rect.width); // NOLINT
            child->rect.x      = next_pos_x;
            next_pos_x        += child->rect.width;
        } else {
            child->rect.height = std::round(mult * parent_rect.height); // NOLINT
            child->rect.width  = parent_rect.width;
            child->rect.y      = next_pos_y;
            next_pos_y        += child->rect.height;
        }
        if (child->leaf()) {
            Window* win = dynamic_cast<Window_container*>(child)->window();
            // It should do calculation for border size and decorations
            win->rect = child->rect;
            XWrap::configure_window(*win);
        } else {
            update_container_recurse(child);
        }
    }
}

void Workspace_manager::update_container(Container* con)
{
    update_container_recurse(con);       
}
