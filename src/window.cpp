#include "window.h"
#include "layout.h"
#include "workspace.h"
#include "logger.h"
#include "frame.h"

// For Window::X11_property implementation.
#include "x11/window.h"

void Window_list::add(Window& window)
{
    logger::debug("Window list -> adding window: {:#x}", window.index());

    _list.push_back(&window);
}

void Window_list::focus(const_iterator it)
{
    logger::debug("Window list -> focusing window: {:#x}", (it->index()));

    if (_list.back()->focused()) _list.back()->unfocus();
    Window& window = *it;
    _list.erase(it);
    _list.push_back(&window);
    window.focus();
}

void Window_list::remove(const_iterator it)
{
    logger::debug("Window list -> removing window: {:#x}", it->index());

    if (it->focused()) it->unfocus();
    _list.erase(it);
}

Window::Window(unsigned int id, Window::Display_type dt) noexcept
    : Managed(id)
    , _display_type(dt)
    , _layout_mark(Layout_mark(*this))
{
    assert(_frame);
}

bool Window::is_marked() const noexcept
{
    if (this->parent().has_value()) {
        const auto &parent = this->parent()->get().get<Layout>();
        return (_layout_mark._marked) && (_layout_mark._type != parent.type());
    } else return false;
}

void Window::_fill_xprop(memory::owner<Window::X11_property> xprop) noexcept
{ _xprop = std::move(xprop); }

void Window::_fill_frame(memory::owner<Window_frame> frame) noexcept
{ _frame = std::move(frame); }

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
    auto it = std::ranges::find(window_list, window);
    assert(it != window_list.cend());
    window_list.focus(it);
}

void focus_last(Window_list& window_list)
{
    if (!window_list.empty())
        window_list.focus(std::ranges::prev(window_list.cend()));
}

void try_focus_window(Window& window)
{
    auto& window_list = window.root<Workspace>().window_list();
    assert(!window_list.empty());
    if (window != window_list.current()->get()) {
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
        Layout *con = new Layout(Layout::Type::Horizontal);
        con->add(window);
        workspace.add(*con);
        workspace.update_rect();
    } else {
        // This must exist, if not, then there's no focused window on workspace
        // While there's one. This is not permissible.
        assert(workspace.window_list().current());
        auto &current_win = workspace.window_list().current()->get();

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
    Layout::Type mark = layout_mark.type();

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
    if (node.size() == 1
        && (!node.back()->get().is_leaf() || !parent.is_root())) {
        Node<Container> &back = node.back()->get();
        node.erase(node.begin());
        // Iterator invalidation
        parent.insert(std::ranges::find(parent, node), back);
        parent.erase(std::ranges::find(parent, node));
        delete &node;

        parent.update_rect();
    } else return false;
    return true;
}

} // namespace window