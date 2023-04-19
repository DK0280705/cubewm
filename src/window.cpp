#include "window.h"
#include "layout.h"
#include "window.h"
#include "workspace.h"
#include "logger.h"

void place(Window& window, Workspace& workspace)
{
    logger::debug("Placing window {:x} to workspace {}", window.index(), workspace.index());
    if (workspace.empty()) {
        // By default it's a horizontal container
        // Maybe i will add a config to change default container
        Layout* con = new Layout(Layout::Type::Horizontal);
        con->add(window);
        workspace.add(*con);
        workspace.update_rect();
    } else {
        // This must exist, if not, then there's no focused window on workspace
        // While there's one. This is not permissible.
        assert(workspace.window_list().current());
        auto& current_win = workspace.window_list().current()->get();

        // Don't add window to focus list before calling this function.
        assert(&current_win != &window);
        assert(current_win.parent());
        auto& parent = static_cast<Layout&>(current_win.parent()->get());
        const auto& markref = current_win.marked_as_new_layout();
        if (markref and markref.value() != parent.type()) {
            if (parent.size() > 1) {
                Layout* con = new Layout(markref.value());
                parent.insert(std::ranges::find(parent, current_win), *con);
                transfer(current_win, *con);
                con->add(window);
                parent.update_rect();
            } else {
                parent.add(window);
                parent.type(markref.value());
            }
        } else {
            parent.add(window);
            parent.update_rect();
        }
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
        return Vector2D::contains(win.rect(), pos);
    });
    return (it != ws.window_list().end()) ? std::optref<Window>(*it) : std::nullopt;
}