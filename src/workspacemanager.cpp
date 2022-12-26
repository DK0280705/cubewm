#include "workspacemanager.h"
#include "container.h"
#include "error.h"
#include "workspace.h"

Workspace_manager::Workspace_manager(Server& srv)
    : Manager(srv)
    , _focused_container(nullptr)
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
    _focused_container = con;
}

void Workspace_manager::place_container(Container* con)
{
    switch (focused()->type) {
    case Container::CT_WORKSPACE:
        focused()->add_child(con);
        break;
    case Container::CT_CONTAINER:
        if (focused()->orientation == focused()->parent()->orientation)
            focused()->parent()->add_child(con);
        else {
            Container* split_con = new Container();
            Container* parent    = focused()->parent();
            split_con->transfer_child(focused())
                     ->add_child(con);
            parent->add_child(split_con);
        }
        break;
    default:
        assert_runtime(false, "What? you focused a dock container?");
        break;
    }
    focus(con);
}

void Workspace_manager::purge_container(Container* con)
{
    // If you try to purge workspace,
    // you know if you know
    Container* parent = con->parent();
    parent->remove_child(con);
    
    // Check if there's only one container in the parent
    // Transfer that container into the parent of the parent
    if (parent->type == Container::CT_CONTAINER && parent->size() == 1) {
        Container* grand_parent = parent->parent();
        grand_parent->transfer_child(parent->front())
                    ->remove_child(parent);
        delete parent;
    } 
}
