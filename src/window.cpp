#include "window.h"
#include "config.h"
#include "layout.h"
#include "window.h"
#include "workspace.h"
#include "logger.h"

void Window::update_rect() noexcept
{
    _actual_size = {
        { this->rect().pos.x + (int)config::gap_size, this->rect().pos.y + (int)config::gap_size },
        { this->rect().size.x - 2*(int)config::gap_size, this->rect().size.y - 2*(int)config::gap_size }
    };
    frame().rect(this->rect());
    _update_rect();
}

bool Window::is_marked() const noexcept
{
    if (!parent().has_value()) return false;
    const auto& parent = this->parent()->get().get<Layout>();
    return (_layout_mark._marked) && (_layout_mark._type != parent.type());
}

void move_to_workspace(Window& window, Workspace& workspace)
{
    logger::debug("Placing window {:x} to workspace {}", window.index(), workspace.index());

    // Remove existing parent before moving.
    if (window.parent()) purge_and_reconfigure(window);

    // Meaning its only has floating layout container.
    if (workspace.size() == 1) {
        // By default it's a horizontal container
        // Maybe i will add a config to change default container
        Layout* con = new Layout(Layout::Type::Horizontal);
        con->add(window);
        workspace.add(*con);
        workspace.update_rect();
    } else {
        // This must exist, if not, then there's no focused window on workspace
        // While there's one. This is not permissible.
        assert(workspace.window_list().last());
        auto& current_win = workspace.window_list().last()->get();

        // Don't add window to focus list before calling this function.
        assert(current_win != window);
        assert(current_win.parent());
        if (auto lm = current_win.layout_mark()) {
            move_to_marked_window(window, lm.value());
        } else {
            auto& parent     = current_win.parent()->get();
            auto  current_it = std::ranges::find(parent, current_win);
            parent.insert(std::ranges::next(current_it), window);
            parent.update_rect();
        }
    }
}

void move_to_marked_window(Window& window, Window::Layout_mark& layout_mark)
{
    if (window.parent()) purge_and_reconfigure(window);

    Window& marked_window = layout_mark.window();
    Layout::Type mark     = layout_mark.type();

    assert(marked_window.parent());
    Layout& parent = marked_window.parent()->get().get<Layout>();

    // If parent has one child, just change the type of layout.
    if (parent.size() > 1) {
        Layout* layout = new Layout(mark);
        parent.insert(std::ranges::find(parent, marked_window), *layout);
        parent.remove(marked_window);
        layout->add(marked_window);
        layout->add(window);
    } else {
        parent.add(window);
        parent.type(mark);
    }

    parent.update_rect();
}

void purge_and_reconfigure(Window& window)
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
    } else if (!purge_sole_node(parent))
        parent.update_rect();
}

bool purge_sole_node(Node<Container>& node)
{
    assert(node.parent());
    auto& parent = node.parent()->get();

    // If node has one child, move its child into its parent.
    if (node.size() == 1
    && (!node.back()->get().is_leaf() || !parent.is_root())) {
        Node<Container>& back = node.back()->get();
        node.erase(node.begin());
        // Iterator invalidation
        parent.insert(std::ranges::find(parent, node), back);
        parent.erase(std::ranges::find(parent, node));
        delete &node;

        parent.update_rect();
    } else return false;
    return true;
}