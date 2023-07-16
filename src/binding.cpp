#include "binding.h"
#include "geometry.h"
#include "layout.h"
#include "logger.h"
#include "node.h"
#include "state.h"
#include "window.h"

namespace binding {
// debug only
// Will output single cryptic string
// TODO(dk_028): Write a better algorithm to display tree on terminal.
#ifndef NDEBUG

static void _tree_debug(std::string& str, const Node<Container>& node)
{
    uint32_t i = 0;
    if (!node.empty()) str += "-> ";
    for (const auto& c : node) {
        if (!c.is_leaf() && !c.is_root()) {
            const auto& layout = c.get<Layout>();
            switch (layout.type()) {
                case Layout::Containment_type::Horizontal:
                    str += 'H';
                    break;
                case Layout::Containment_type::Vertical:
                    str += 'V';
                    break;
                case Layout::Containment_type::Tabbed:
                    str += 'T';
                    break;
                default:
                    break;
            }
        } else if (c.is_leaf()) str += 'W';
        str += std::to_string(i);
        str += ' ';
        _tree_debug(str, c);
        i++;
    }
    if (!node.empty()) str += " _ ";
}

#endif

template<Direction prev, Direction next> requires(prev != next)
static auto _get_node_pos_by_direction_impl(const Node<Container>& node, Direction dir) noexcept
    -> Node<Container>::const_iterator
{
    const auto& parent = node.parent_unsafe().get<Layout>();
    const auto node_it = std::ranges::find(parent, node);

    switch (dir) {
        case prev:
            return std::ranges::prev(node_it);
        case next:
            return std::ranges::next(node_it);
        default:
            return parent.end();
    }
}

static auto _get_node_pos_by_direction(const Node<Container>& node, Direction dir) noexcept
    -> Node<Container>::const_iterator
{
    assert(node.parent());
    assert(!node.parent_unsafe().is_root());
    const auto& parent = node.parent_unsafe().get<Layout>();

    switch (parent.type()) {
        case Layout::Containment_type::Horizontal:
        case Layout::Containment_type::Tabbed:
            return _get_node_pos_by_direction_impl<Direction::Left, Direction::Right>(node, dir);
        case Layout::Containment_type::Vertical:
            return _get_node_pos_by_direction_impl<Direction::Up, Direction::Down>(node, dir);
        default:
            return parent.cend();
    }
}

static bool _is_layout_type_direction_compatible(Layout::Containment_type type, Direction dir)
{
    switch (dir) {
        case Direction::Left:
        case Direction::Right:
            return (type == Layout::Containment_type::Horizontal
                    || type == Layout::Containment_type::Tabbed);
        case Direction::Up:
        case Direction::Down:
            return (type == Layout::Containment_type::Vertical);
    }
    std::unreachable();
}

static auto _find_direction_compatible_node(Node<Container>& node, Direction dir) noexcept
    -> std::optref<Node<Container>>
{
    assert(node.parent());
    assert(!node.parent_unsafe().is_root());
    auto *current = &node;

    // If the implementation consistent, it will only loop 2 times max.
    while (true) {
        auto& parent = current->parent_unsafe().get<Layout>();
        if (!_is_layout_type_direction_compatible(parent.type(), dir)) {
            if (parent.parent() && !parent.parent()->get().is_root()) {
                current = &parent;
            } else return std::nullopt;
        } else {
            if (parent.size() == 1) return std::nullopt;
            else return *current;
        }
    }
}

static auto _get_current_focused_container(State& state) noexcept
    -> std::optref<Node<Container>>
{
    if (state.current_workspace().has_window())
        return state.current_workspace().current_window();
    else return std::nullopt;
}

// Only use this function to improve readability
static inline void _direction_action(auto&& prev_fn, auto&& next_fn, Direction dir)
{
    switch (dir) {
    case Direction::Up:
    case Direction::Left:
        prev_fn();
        break;
    case Direction::Down:
    case Direction::Right:
        next_fn();
        break;
    default:
        std::unreachable();
    }
};

void Move_focus::execute(State& state) const noexcept
{
    const auto& objref = _get_current_focused_container(state);
    if (!objref.has_value()) return;

    Node<Container>& object = objref->get();

    if (const auto& noderef = _find_direction_compatible_node(object, _dir)) {
        const auto& node = noderef->get();
        const auto& parent = node.parent_unsafe();

        auto adj_it = _get_node_pos_by_direction(node, _dir);
        if (adj_it == parent.cend()) {
            switch (_dir) {
                case Direction::Up:
                case Direction::Left:
                    adj_it = std::ranges::prev(parent.cend());
                    break;
                case Direction::Down:
                case Direction::Right:
                    adj_it = parent.cbegin();
                    break;
            }
        }
        state.current_workspace().focus_window(get_front_leaf(*adj_it).get<Window>());
    }
}

static void _move_to_adjacent_node(Window& window, Node<Container>& node, Direction dir) noexcept
{
    assert(node.parent());
    auto& parent = node.parent_unsafe().get<Layout>();
    auto node_it = std::ranges::find(parent, node);

    // Unmark window if marked.
    window.unmark_layout();

    // Get adjacent node according to direction.
    auto adj_it = _get_node_pos_by_direction(node, dir);

    // If adjacent node not available, send window to the begin/end of its parent node.
    if (adj_it == parent.cend()) {
        if (parent.type() == window.parent_unsafe().get<Layout>().type())
            return;
        _direction_action(
                [&]() { window::move_to_layout(window, parent, parent.cbegin()); },
                [&]() { window::move_to_layout(window, parent, parent.cend()); },
                dir);

        // This means, adjacent node is available
    } else if (adj_it->is_leaf()) {
        // Adjacent node is a window.
        if (auto lm = adj_it->get<Window>().layout_mark()) {
            // Create a new layout with marked window.
            window::move_to_marked_window(window, lm.value());
        } else {
            if (parent == window.parent_unsafe()) {
                _direction_action(
                        [&]() { parent.shift_child_backward(node_it); },
                        [&]() { parent.shift_child_forward(node_it); },
                        dir);
                parent.update_rect();
            } else {
                _direction_action(
                        [&]() { window::move_to_layout(window, parent, node_it); },
                        [&]() { window::move_to_layout(window, parent, adj_it); },
                        dir);
            }
        }
    } else {
        // Adjacent node is a layout here. Put window into it.
        window::move_to_layout(window, adj_it->get<Layout>());
    }
}

static inline auto _get_inverse_layout_type(Layout::Containment_type type) noexcept
    -> Layout::Containment_type
{
    switch (type) {
    case Layout::Containment_type::Horizontal:
    case Layout::Containment_type::Tabbed:
        return Layout::Containment_type::Vertical;
    case Layout::Containment_type::Vertical:
        return Layout::Containment_type::Horizontal;
    default: std::unreachable();
    }
}

static void _move_to_top_node(Window& window, Workspace& workspace, Direction dir) noexcept
{
    // Purge window from its parent first
    window::purge_and_reconfigure(window);

    // Get the top tiling layout.
    Layout& top_layout = workspace.tiling_layout();
    // If layout type and direction compatible, simply add the window.
    if (_is_layout_type_direction_compatible(top_layout.type(), dir)) {
        top_layout.add_child(window);
        // If top layout size is only one, better change its type.
    } else if (top_layout.size() == 1) {
        top_layout.type(_get_inverse_layout_type(top_layout.type()));
        _direction_action(
                [&]() { top_layout.insert_child(top_layout.begin(), window); },
                [&]() { top_layout.add_child(window); },
                dir);
    } else {
        // Silent this stupid warning, if new fails, just let the program go boom.
        auto* layout = new Layout(_get_inverse_layout_type(top_layout.type()));
        workspace.remove_child(top_layout);

        switch (dir) {
            case Direction::Up:
            case Direction::Left:
                layout->add_child(window);
                layout->add_child(top_layout);
                break;
            case Direction::Down:
            case Direction::Right:
                layout->add_child(top_layout);
                layout->add_child(window);
                break;
        }

        workspace.add_child(*layout);
    }
    workspace.update_rect();
}

void Move_container::execute(State& state) const noexcept
{
    if (state.current_workspace().has_window()) {
        Window& window = state.current_workspace().current_window();

        if (const auto& noderef = _find_direction_compatible_node(window, _dir)) {
            assert(noderef->get().parent());
            logger::debug("Move container -> can be moved to its adjacent");
            _move_to_adjacent_node(window, noderef->get(), _dir);
        } else {
            Layout& top_layout = state.current_workspace().tiling_layout();
            if (top_layout.size() == 1) return;

            switch (top_layout.type()) {
                case Layout::Containment_type::Horizontal:
                case Layout::Containment_type::Tabbed:
                    if (_dir == Direction::Right || _dir == Direction::Left) return;
                    break;
                case Layout::Containment_type::Vertical:
                    if (_dir == Direction::Up || _dir == Direction::Down) return;
                    break;
                default: std::unreachable();
            }

            logger::debug("Move container -> can be moved to top node");
            _move_to_top_node(window, state.current_workspace(), _dir);
        }
    }
#ifndef NDEBUG
    std::string str;
    _tree_debug(str, state.current_workspace());
    logger::debug("Move container -> Tree: R{} {}", state.current_workspace().name(), str);
#endif
}

void Change_layout_type::execute(State& state) const noexcept
{
    if (state.current_workspace().has_window()) {
        logger::debug("Change layout type -> type: {}", layout_type_to_str(_change_to));

        Window& window = state.current_workspace().current_window();
        window.mark_layout(_change_to);
    }
}

void Switch_workspace::execute(State& state) const noexcept
{
    if (state.current_workspace().index() != _workspace_id)
        state.switch_workspace(state.get_or_create_workspace(_workspace_id));
}

} // namespace binding
