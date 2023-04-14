#include "window.h"
#include "node.h"
#include "window.h"
#include "workspace.h"
#include "logger.h"

void place_to(Workspace& ws, Window& window, bool create_new)
{
    logger::debug("Placing window {:x} to workspace {}", window.index(), ws.index());
    if (ws.empty() || create_new) {
        // By default it's a horizontal container
        // Maybe i will add a config to change default container
        Layout_container* con = new Horizontal_container();
        con->add(window);
        ws.add(*con);
        ws.update_rect();
    } else {
        // This must exist, if not, then there's no focused window on workspace
        // While there's one. This is not permissible.
        assert(ws.window_list().current());
        auto& win = ws.window_list().current()->get();

        // Don't add window to focus list before calling this function.
        assert(&win != &window);
        assert(win.parent());
        auto& parent = win.parent()->get();
        parent.add(window);
        parent.update_rect();
    }
}

void purge(Window& window)
{
    assert(window.parent());
    auto& parent = window.parent()->get();
    parent.remove(window);

    // TODO: Fix this code smell
    if (parent.empty()) {
        assert(parent.parent());
        auto& gparent = parent.parent()->get();
        gparent.remove(parent);
        delete &parent;
        gparent.update_rect();
    }
    else if (parent.size() == 1 and !parent.front()->get().is_leaf()) {
        assert(parent.parent());
        auto& gparent = parent.parent()->get();
        transfer(parent.front()->get(), gparent);
        delete &parent;
        gparent.update_rect();
    } else {
        parent.update_rect();
    }
}

std::optref<Window> find_window_by_position(Workspace& ws, const Point2D& pos)
{
    auto it = std::ranges::find_if(ws.window_list(), [&](const auto& win){
        return Vector2D::contains(win->rect(), pos);
    });
    return (it != ws.window_list().end()) ? std::optref<Window>(*(*it)) : std::nullopt;
}