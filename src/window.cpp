#include "window.h"
#include "layout.h"
#include "workspace.h"
#include "logger.h"

// For Window::Impl implementation.
#include "x11/window.h"

void Window_list::add(Window& window)
{
    if (!empty() && current().focused())
        current().unfocus();
    _list.push_back(&window);
}

void Window_list::focus(const_iterator it)
{
    assert(!empty());
    if (_list.back()->focused())
        _list.back()->unfocus();

    Window& window = *it;
    _list.erase(it);
    _list.push_back(&window);
    window.focus();
}

void Window_list::remove(const_iterator it)
{
    if (it->focused())
        it->unfocus();

    _list.erase(it);
}


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

Window::~Window() noexcept = default;



namespace window {

void add_window(Window_list& window_list, Window& window)
{
    assert(&window.root<Workspace>().window_list() == &window_list);
    assert(std::ranges::find(window_list, window) == window_list.cend());
    window_list.add(window);
}

void focus_window(Window_list& window_list, Window& window)
{
    if (window != window_list.current()) {
        auto it = std::ranges::find(window_list, window);
        assert(it != window_list.cend());
        window_list.focus(it);
    }
}

void focus_last(Window_list& window_list)
{
    if (!window_list.empty())
        window_list.focus(std::ranges::prev(window_list.cend()));
}

void try_focus_window(Window& window)
{
    if (window.focused()) return;
    auto& window_list = window.root<Workspace>().window_list();
    assert(!window_list.empty());
    if (window != window_list.current()) {
        auto it = std::ranges::find(window_list, window);
        // This FAILS if try to focus before add_window().
        assert(it != window_list.cend());
        window_list.focus(it);
    }
}

void remove_window(Window_list& window_list, Window& window)
{
    auto it = std::ranges::find(window_list, window);
    assert(it != window_list.cend());
    window_list.remove(it);
}

void move_to_workspace(Window &window, Workspace &workspace)
{
    logger::debug("Placing window {:x} to workspace {}", window.index(), workspace.index());

    // Remove existing parent before moving.
    if (window.parent()) purge_and_reconfigure(window);

    // Meaning its only has floating layout container.
    if (workspace.size() == 1) {
        // By default it's a horizontal container
        // Maybe i will add a config to change default container
        Layout *con = new Layout(Layout::Containment_type::Horizontal);
        con->add(window);
        workspace.add(*con);
        workspace.update_rect();
    } else {
        // This must exist, if not, then there's no focused window on workspace
        // While there's one. This is not permissible.
        assert(!workspace.window_list().empty());
        auto &current_win = workspace.window_list().current();

        // Don't add window to focus list before calling this function.
        assert(current_win != window);
        assert(current_win.parent());
        if (auto lm = current_win.layout_mark()) {
            move_to_marked_window(window, lm.value());
        } else {
            auto &parent = current_win.parent()->get();
            auto current_it = std::ranges::find(parent, current_win);
            parent.insert(std::ranges::next(current_it), window);
            parent.update_rect();
        }
    }
}

void move_to_marked_window(Window &window, Window::Layout_mark &layout_mark)
{
    if (window.parent()) purge_and_reconfigure(window);

    Window &marked_window = layout_mark.window();
    Layout::Containment_type mark = layout_mark.type();

    assert(marked_window.parent());
    auto& parent = marked_window.parent()->get().get<Layout>();

    // If parent has one child, just change the type of layout.
    if (parent.size() > 1) {
        auto* layout = new Layout(mark);
        parent.insert(std::ranges::find(parent, marked_window), *layout);
        parent.remove(marked_window);
        layout->add(marked_window);
        layout->add(window);
        parent.update_rect();
    } else {
        parent.add(window);
        parent.type(mark);
    }
}

void purge_and_reconfigure(Window &window)
{
    assert(window.parent());
    auto &parent = window.parent()->get();
    parent.remove(window);

    if (parent.empty()) {
        assert(parent.parent());
        auto &gparent = parent.parent()->get();

        gparent.remove(parent);
        delete &parent;

        gparent.update_rect();

    } else if (!purge_sole_node(parent))
        parent.update_rect();
}

bool purge_sole_node(Node<Container> &node)
{
    assert(node.parent());
    auto &parent = node.parent()->get();

    // If node has one child, move its child into its parent.
    if ((node.size() == 1) && (!node.back().is_leaf() || !parent.is_root())) {
        Node<Container> &back = node.back();
        node.erase(node.begin());
        // Iterator invalidation, so search twice.
        parent.insert(std::ranges::find(parent, node), back);
        parent.erase(std::ranges::find(parent, node));
        delete &node;

        parent.update_rect();
    } else return false;
    return true;
}

} // namespace window
