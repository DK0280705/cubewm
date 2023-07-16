#include "window.h"
#include "layout.h"
#include "workspace.h"
#include "logger.h"

// For Window::Impl implementation.
#include "x11/window.h"


Window::Window(unsigned int id, Display_type dt)
    : Managed(id)
    , _display_type(dt)
    , _window_state(Window::State::Normal)
    , _placement_mode(Placement_mode::Tiling)
    , _layout_mark(Layout_mark(*this))
{
    switch (dt) {
    case Display_type::X11: {
        _impl = memory::make_owner<X11::Window_impl>(*this);
        auto geometry = X11::window::get_geometry(index());
        assert_runtime((bool)geometry, "Window's geometry null");
        this->rect({
            { geometry->x, geometry->y },
            { geometry->width, geometry->height }
        });
        break;
    }
    default:
        assert_runtime(false, "Unknown display");
    }
}

void Window::_update_rect_fn() noexcept
{
    _impl->update_rect();
}

void Window::_update_focus_fn() noexcept
{
    _impl->update_focus();
}

bool Window::is_marked() const noexcept
{
    if (this->parent().has_value()) {
        const auto &parent = this->parent()->get().get<Layout>();
        return (_layout_mark._type != Layout::Containment_type::Floating)
            && (_layout_mark._type != parent.type());
    } else return false;
}

void Window::normalize() noexcept
{
    if (_window_state != Window::State::Normal) {
        _window_state = Window::State::Normal;
        _impl->update_state(Window::State::Normal);
    }
}

void Window::minimize() noexcept
{
    if (_window_state != Window::State::Minimized) {
        _window_state = Window::State::Minimized;
        _impl->update_state(Window::State::Minimized);
    }
}

void Window::maximize() noexcept
{
    if (_window_state != Window::State::Maximized) {
        _window_state = Window::State::Maximized;
        _impl->update_state(Window::State::Maximized);
    }
}

void Window::set_tiling() noexcept
{
    _placement_mode = Placement_mode::Tiling;
    normalize();
}

void Window::set_floating() noexcept
{
    _placement_mode = Placement_mode::Floating;
    normalize();
}

void Window::kill() noexcept
{
    _impl->kill();
}

Window::~Window() noexcept = default;



namespace window {

void try_focus_window(Window& window)
{
    if (window.focused()) return;
    auto& workspace = window.root<Workspace>();
    workspace.focus_window(window);
}

void move_to_workspace(Window& window, Workspace& workspace)
{
    assert_debug(window.placement_mode() != Window::Placement_mode::Sticky, "Sticky window should not be added to workspace.");

    logger::debug("Placing window {:x} to workspace {}", window.index(), workspace.index());
    bool mapped = window.parent().has_value();
    // Remove existing parent before moving.
    if (mapped) {
        auto& last_workspace = window.root<Workspace>();
        // Last workspace should not be the same as current workspace.
        if (last_workspace == workspace) return;
        last_workspace.remove_window(window);
        purge_and_reconfigure(window);
    }

    switch(window.placement_mode()) {
    case Window::Placement_mode::Floating:
    {
        auto& floating_layout = workspace.floating_layout();
        floating_layout.add_child(window);
        break;
    }
    case Window::Placement_mode::Tiling:
    {
        // Meaning its only has floating layout container.
        if (workspace.size() == 1) {
            // By default it's a horizontal container
            // Maybe i will add a config to change default container
            auto *con = new Layout(Layout::Containment_type::Horizontal);
            con->add_child(window);
            workspace.add_child(*con);
            workspace.update_rect();
        } else {
            auto &current_window = workspace.current_window();

            // Don't add window to focus list before calling this function.
            assert(current_window != window);
            if (auto lm = current_window.layout_mark()) {
                move_to_marked_window(window, lm.value());
            } else {
                auto &parent = current_window.parent()->get();
                auto it      = std::ranges::find(parent, current_window);
                parent.insert_child(std::ranges::next(it), window);
                parent.update_rect();
            }
        }
        break;
    }
    default: std::unreachable();
    }

    workspace.add_window(window);
}

void move_to_marked_window(Window& window, Window::Layout_mark& layout_mark)
{
    if (window.parent()) purge_and_reconfigure(window);

    Window &marked_window = layout_mark.window();
    Layout::Containment_type mark = layout_mark.type();

    assert(marked_window.parent());
    auto& parent = marked_window.parent()->get().get<Layout>();

    // If parent has one child, just change the type of layout.
    if (parent.size() > 1) {
        auto* layout = new Layout(mark);
        parent.insert_child(std::ranges::find(parent, marked_window), *layout);
        parent.remove_child(marked_window);
        layout->add_child(marked_window);
        layout->add_child(window);
        parent.update_rect();
    } else {
        parent.add_child(window);
        parent.type(mark);
    }
}

// returns false if fails.
static bool _purge_sole_node(Node<Container>& node)
{
    assert(node.parent());
    auto& parent = node.parent()->get();

    // If node has one child, move its child into its parent.
    if ((node.size() == 1) && (!node.back().is_leaf() || !parent.is_root())) {
        Node<Container>& back = node.back();
        node.erase_child(node.begin());
        // Iterator invalidation, so search twice.
        parent.insert_child(std::ranges::find(parent, node), back);
        parent.erase_child(std::ranges::find(parent, node));
        delete &node;

        parent.update_rect();
    } else return false;
    return true;
}

void move_to_layout(Window& window, Layout& layout)
{
    move_to_layout(window, layout, layout.cend());
}

void move_to_layout(Window& window, Layout& layout, Layout::const_iterator pos)
{
    auto& parent = window.parent_unsafe();
    parent.remove_child(window);
    layout.insert_child(pos, window);
    _purge_sole_node(parent);
    layout.parent_unsafe().update_rect();
}

void purge_and_reconfigure(Window& window)
{
    assert(window.parent());
    auto& parent = window.parent()->get();
    parent.remove_child(window);

    if (parent.empty()) {
        assert(parent.parent());
        auto& gparent = parent.parent()->get();

        gparent.remove_child(parent);
        delete &parent;

        gparent.update_rect();

    } else if (!_purge_sole_node(parent))
        parent.update_rect();
}

} // namespace window
