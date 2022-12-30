#include "container.h"
#include "error.h"
#include "window.h"
#include "workspace.h"
#include <algorithm>

Container::Container() noexcept
    : orientation(CO_HORIZONTAL)
    , type(CT_CONTAINER)
    , rect({0, 0, 0, 0})
    , floating(false)
    , hidden(false)
    , _parent(nullptr)
{
}

Container* Container::add_child(Container* con, Container* next_to)
{
    con->_parent = this;
    if (!next_to)
        _children.push_back(con);
    else {
        const auto it = std::find(_children.begin(), _children.end(), next_to);
        _children.insert(std::next(it, 1), con);
    }
    return this;
}

Container* Container::transfer_child(Container* con)
{
    con->_parent->_children.remove(con);
    return add_child(con);
}

Container* Container::remove_child(Container* con)
{
    con->_parent = nullptr;
    _children.remove(con);
    return this;
}

Workspace* Container::get_workspace()
{
    assert_runtime(type != CT_DOCKAREA, "Invalid container");
    return _parent->get_workspace();
}

Container::~Container()
{
    for (const auto& c : _children) delete c;
}

Window_container::Window_container(Window* win)
    : _window(win)
{
    win->_container = this;
}
