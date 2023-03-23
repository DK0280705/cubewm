#include "window.h"
#include "node.h"
#include "window.h"
#include "workspace.h"
#include "logger.h"

void place_to(Workspace* ws, Window* window, bool create_new)
{
    logger::debug("Placing window {:x} to workspace {}", window->index(), ws->index());
    window->workspace(ws);
    if (ws->empty() || create_new) {
        // By default it's a horizontal container
        // Maybe i will add a config to change default container
        Layout_container* con = new Horizontal_container(ws);
        ws->add(con);
        con->add(window);
        ws->update_rect();
    } else {
        // This must exist, if not, then there's no focused window on workspace
        // While there's one. This is not permissible.
        auto* win = ws->window_list().current();
        assert(win);

        // Don't add window to focus list before calling this function.
        assert(win != window);

        auto* parent = win->parent();
        parent->add(window);
        parent->update_rect();
    }
}

void purge(Window* window)
{
    auto* parent = window->parent();
    parent->remove(window);

    auto* gparent = parent->parent();
    if (parent->empty()) {
        gparent->remove(parent);
        delete parent;
        gparent->update_rect();
    } else if(parent->size() == 1 && !(*parent->begin())->empty()) {
        transfer_to(gparent, *parent->begin());
        delete parent;
        gparent->update_rect();
    } else {
        parent->update_rect();
    }
}

Window* find_window_by_position(Workspace* ws, const Point2D& pos)
{
    for (const auto& win : ws->window_list()) {
        const auto& crect = win->rect();
        if ((crect.pos.x <= pos.x && crect.size.x >= pos.x) &&
            (crect.pos.y <= pos.y && crect.size.y >= pos.y))
            return win;
    }
    return nullptr;
}