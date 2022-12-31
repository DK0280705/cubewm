#include "container.h"
#include "error.h"
#include "window.h"
#include "workspace.h"
#include "xwrap.h"
#include <cmath>
#include <algorithm>

Container::Container() noexcept
    : _type(CT::Container)
    , _orientation(CO::Horizontal)
    , _rect({0, 0, 0, 0})
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
    assert_runtime(_type != CT::Dockarea, "Invalid container");
    return _parent->get_workspace();
}

void Container::configure_child_rect()
{
    uint32_t next_pos_x = _rect.x;
    uint32_t next_pos_y = _rect.y;

    for (const auto& child : _children) {
        // All of the configured resize will be reset
        const float mult = 1.0f / static_cast<float>(size());
        if (_orientation == CO::Horizontal) {
            child->_rect.height = _rect.height;
            child->_rect.width  = std::round(mult * _rect.width); // NOLINT
            child->_rect.x      = next_pos_x;
            next_pos_x         += child->_rect.width;
        } else {
            child->_rect.height = std::round(mult * _rect.height); // NOLINT
            child->_rect.width  = _rect.width;
            child->_rect.y      = next_pos_y;
            next_pos_y        += child->_rect.height;
        }
        child->configure_child_rect();
    }
}

Container::~Container()
{
    for (const auto& c : _children) delete c;
}

Window_container::Window_container(Window* win)
    : _window(win)
{
    win->container = this;
}

void Window_container::configure_child_rect()
{
    // It should do calculation for border size and decorations
    _window->rect = _rect;
    XWrap::configure_window(*_window);
}
