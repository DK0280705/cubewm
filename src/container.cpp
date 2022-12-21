#include "container.h"
#include "workspace.h"
#include "error.h"

Container::Container(Server& srv)
    : srv(srv)
{
}

Container& Container::add_child(Container* con)
{
    children.push_back(con);
    return *this;
}

Container& Container::transfer_child(Container* con)
{
    con->parent->children.remove(con);
    return add_child(con);
}

Container& Container::remove_child(Container* con)
{
    children.remove(con);
    delete con;
    return *this;
}

Workspace* Container::get_workspace()
{
    assert_runtime(type != CT_DOCKAREA, "Invalid container");
    return parent->get_workspace();
}

Container::~Container()
{
    for (const auto& c : children)
        delete c;
}
