#include "window.h"
#include "layout.h"
#include "window.h"
#include "workspace.h"
#include "logger.h"

// A window must be contained in a Layout container
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
        if (!move_to_marked_window(window, current_win)) {
            auto current_it = std::ranges::find(current_win.parent()->get(), current_win);
            current_win.parent()->get().insert(std::ranges::next(current_it), window);
            current_win.parent()->get().update_rect();
        }
    }
}

bool move_to_marked_window(Window& window, Window& marked_window)
{
    assert(marked_window.parent());
    auto& parent = static_cast<Layout&>(marked_window.parent()->get());

    const auto& markref = marked_window.layout_mark();
    if (markref and markref != parent.type()) {
        if (window.parent()) purge(window);
        if (parent.size() > 1) {
            Layout* layout = new Layout(markref.value());
            parent.insert(std::ranges::find(parent, marked_window), *layout);
            parent.remove(marked_window);
            layout->add(marked_window);
            layout->add(window);
        } else {
            parent.add(window);
            parent.type(markref.value());
        }
        parent.update_rect();
    } else return false;
    return true;
}

void purge(Window& window, bool rebase)
{
    assert(window.parent());
    auto& parent = window.parent()->get();
    parent.remove(window);

    if (parent.empty()) {
        assert(parent.parent());
        auto& gparent = parent.parent()->get();
        gparent.remove(parent);
        delete &parent;
        gparent.update_rect();
    } else if (!rebase or !purge_sole_node(parent))
        parent.update_rect();
}

bool purge_sole_node(Node<Container>& node)
{
    assert(node.parent());
    auto& parent = node.parent()->get();
    if (node.size() == 1 and
       (!node.front()->get().is_leaf() or !parent.is_root())) {
        Node<Container>& front = node.front()->get();
        node.erase(node.begin());
        // Iterator invalidation
        parent.insert(std::ranges::find(parent, node), front);
        parent.erase(std::ranges::find(parent, node));
        delete &node;

        parent.update_rect();
    } else return false;
    return true;
}

std::optref<Window> find_window_by_position(Workspace& ws, const Point2D& pos)
{
    auto it = std::ranges::find_if(ws.window_list(), [&](const auto& win){
        return Vector2D::contains(win.rect(), pos);
    });
    return (it != ws.window_list().end()) ? std::optref<Window>(*it) : std::nullopt;
}