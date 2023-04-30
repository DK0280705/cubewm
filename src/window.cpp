#include "window.h"
#include "layout.h"
#include "window.h"
#include "workspace.h"
#include "logger.h"

bool is_window_marked(Window& window) noexcept
{
    assert(window.parent());
    const auto& parent  = window.parent()->get().get<Layout>();
    const auto& markref = window.layout_mark();
    return markref && markref.value() != parent.type();
}

auto get_marked_window(Window& window) noexcept
    -> std::optional<Marked_window>
{
    return is_window_marked(window) ? std::optional(Marked_window(window)) : std::nullopt;
}

void move_to_workspace(Window& window, Workspace& workspace)
{
    logger::debug("Placing window {:x} to workspace {}", window.index(), workspace.index());

    // Remove existing parent before moving.
    if (window.parent()) purge_and_reconfigure(window);

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
        assert(workspace.window_list().last());
        auto& current_win = workspace.window_list().last()->get();

        // Don't add window to focus list before calling this function.
        assert(&current_win != &window);
        assert(current_win.parent());
        if (auto mw = get_marked_window(current_win)) {
            move_to_marked_window(window, mw.value());
        } else {
            auto current_it = std::ranges::find(current_win.parent()->get(), current_win);
            current_win.parent()->get().insert(std::ranges::next(current_it), window);
            current_win.parent()->get().update_rect();
        }
    }
}

void move_to_marked_window(Window& window, Marked_window& m_window)
{
    if (window.parent()) purge_and_reconfigure(window);

    Window& marked_window = m_window.window;
    Layout& parent        = marked_window.parent()->get().get<Layout>();
    Layout::Type mark     = marked_window.layout_mark().value();

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
    && (!node.front()->get().is_leaf() || !parent.is_root())) {
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