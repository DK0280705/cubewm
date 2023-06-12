#include "binding.h"
#include "geometry.h"
#include "layout.h"
#include "logger.h"
#include "node.h"
#include "state.h"
#include "window.h"

//TODO: FIX ALL OF THIS MESS

template<Direction prev, Direction next> requires(prev != next)
static auto _get_iter_by_direction(Node<Container>& parent, Node<Container>::const_iterator child_it, Direction dir)
    -> Node<Container>::const_iterator
{
    switch(dir) {
        case prev: return std::ranges::prev(child_it);
        case next: return std::ranges::next(child_it);
        default: return parent.end();
    }
}

static auto _get_iter_by_direction(Layout& parent, Node<Container>::const_iterator child_it, Direction dir) noexcept
    -> Node<Container>::const_iterator
{
    switch (parent.type()) {
    case Layout::Type::Horizontal:
        return _get_iter_by_direction<Direction::Left, Direction::Right>(parent, child_it, dir);
    case Layout::Type::Vertical:
        return _get_iter_by_direction<Direction::Up, Direction::Down>(parent, child_it, dir);
    case Layout::Type::Tabbed:
        return _get_iter_by_direction<Direction::Left, Direction::Right>(parent, child_it, dir);
    default:
        return parent.cend();
    }
}
static bool _type_direction_compatible(Layout::Type type, Direction dir)
{
    switch (dir) {
    case Direction::Left:
    case Direction::Right:
        if (type == Layout::Type::Horizontal
         || type == Layout::Type::Tabbed) return true;
        break;
    case Direction::Up:
    case Direction::Down:
        if (type == Layout::Type::Vertical) return true;
        break;
    }
    return false;
}

static auto _get_type_direction_compatible_node_pair(Node<Container>& node, Direction dir) noexcept
    -> std::optional<std::pair<std::reference_wrapper<Layout>, Node<Container>::const_iterator>>
{
    if (!node.parent() || node.parent()->get().is_root())
        return std::nullopt;
    Node<Container>* current = &node;
    Layout*          parent  = &node.parent()->get().get<Layout>();

    for (int i = 0; ; ++i) {
        logger::debug("Get node -> iteration: {}", i);

        if (!_type_direction_compatible(parent->type(), dir)) {
            if (parent->parent() && !parent->parent()->get().is_root()) {
                current = parent;
                parent  = &parent->parent()->get().get<Layout>();
            } else return std::nullopt;
        } else {
            if (parent->size() == 1) return std::nullopt;
            else return std::optional(std::pair<std::reference_wrapper<Layout>, Node<Container>::const_iterator> (*parent, std::ranges::find(*parent, *current)));
        }
    }
}

static Layout::Type _get_direction_compatible_layout_type(Direction dir)
{
    switch (dir) {
    case Direction::Up:
    case Direction::Down:
        return Layout::Type::Vertical;
    case Direction::Left:
    case Direction::Right:
        return Layout::Type::Horizontal;
    }
    return Layout::Type::Floating; //silent compiler
}

void Move_focus::operator()(State& state) const noexcept
{
    const auto& layref = state.current_workspace().focused_layout();
    Node<Container>* focused_object;
    if (layref) focused_object = &layref->get();
    else {
        if (const auto& winref = state.current_workspace().window_list().current())
            focused_object = &winref->get();
        else return;
    }

    logger::debug("Move focus -> direction: {}", direction_to_str(_dir));

    Node<Container>& object = *focused_object;

    assert(object.parent());
    auto pair_noderef = _get_type_direction_compatible_node_pair(object, _dir);
    if (pair_noderef) {
        auto&[parent, child_it] = pair_noderef.value();

        auto it = _get_iter_by_direction(parent, child_it, _dir);
        if (it == parent.get().cend()) {
            switch (_dir) {
            case Direction::Up:
            case Direction::Left:
                it = std::ranges::prev(parent.get().cend());
                break;
            case Direction::Down:
            case Direction::Right:
                it = parent.get().cbegin();
                break;
            }
        }
        window::focus_window(state.current_workspace().window_list(), get_front_leaf(*it).get<Window>());
    }
}

// debug only
#ifndef NDEBUG
static void _get_tree_numbers(std::string& str, const Node<Container>& node)
{
    uint32_t i = 0;
    if (!node.empty()) str += "-> ";
    for (const auto& c : node) {
        if (!c.is_leaf() && !c.is_root()) {
            const Layout& layout = c.get<Layout>();
            switch (layout.type()) {
            case Layout::Type::Horizontal:
                str += 'H'; break;
            case Layout::Type::Vertical:
                str += 'V'; break;
            case Layout::Type::Tabbed:
                str += 'T'; break;
            default: break;
            }
        } else if (c.is_leaf()) str += 'W';
        str += std::to_string(i);
        str += ' ';
        _get_tree_numbers(str, c);
        i++;
    }
    if (!node.empty()) str += " _ ";
}
#endif

static void _move_to_nearest_neighbour(Window& window,
                                       Layout& parent,
                                       Node<Container>::const_iterator parent_child_it,
                                       Node<Container>::const_iterator neighbour_it,
                                       Direction dir) noexcept
{
    Node<Container>& neighbour = *neighbour_it;
    window.unmark_layout();
    if (neighbour.is_leaf()) {
        if (auto lm = neighbour.get<Window>().layout_mark()) {
            window::move_to_marked_window(window, lm.value());
        } else {
            switch (dir) {
            case Direction::Up:
            case Direction::Left:
            {
                if (parent == window.parent()->get()) {
                    parent.shift_backward(parent_child_it);
                } else {
                    // Prevent iterator invalidation
                    window.parent()->get().remove(window);
                    parent.insert(parent_child_it, window);
                    window::purge_sole_node(*parent_child_it);
                }
                parent.update_rect();
                break;
            }
            case Direction::Down:
            case Direction::Right:
            {
                if (parent == window.parent()->get()) {
                    parent.shift_forward(parent_child_it);
                } else {
                    // Prevent iterator invalidation
                    window.parent()->get().remove(window);
                    parent.insert(neighbour_it, window);
                    window::purge_sole_node(*parent_child_it);
                }
                parent.update_rect();
                break;
            }
            }
        }
    } else {
        window::purge_and_reconfigure(window);
        neighbour.add(window);
        neighbour.update_rect();
    }
}

static void _move_to_top_node(Window& window, Workspace& workspace, Direction dir) noexcept
{
    window::purge_and_reconfigure(window);
    Layout& top  = workspace.tiling_layout();
    auto    type = _get_direction_compatible_layout_type(dir);
    if (top.type() == type) {
        top.add(window);
    } else {
        Layout* layout = new Layout(type);
        workspace.remove(top);
        switch (dir) {
        case Direction::Up:
        case Direction::Left:
            layout->add(window);
            layout->add(top);
            break;
        case Direction::Down:
        case Direction::Right:
            layout->add(top);
            layout->add(window);
            break;
        }
        workspace.add(*layout);
    }
    workspace.update_rect();
}

void Move_container::operator()(State& state) const noexcept
{
    if (const auto& layref = state.current_workspace().focused_layout()) {
        Layout& layout = layref->get();
        if (layout.parent() && layout.parent()->get().is_root()) return;

        Layout& parent = layout.parent()->get().get<Layout>();
        switch (_dir) {
        case Direction::Up:
        case Direction::Left:
            if (parent.front()->get() != layout)
                parent.shift_backward(std::ranges::find(parent, layout));
            break;
        case Direction::Down:
        case Direction::Right:
            if (parent.back()->get() != layout)
                parent.shift_forward(std::ranges::find(parent, layout));
            break;
        }
    } else if (const auto& winref = state.current_workspace().window_list().current()) {
        logger::debug("Move container -> direction: {}", direction_to_str(_dir));

        Window& window = winref->get();
        // TODO: Fix this smell
        assert(window.parent());
        auto pair_noderef = _get_type_direction_compatible_node_pair(window, _dir);
        if (pair_noderef) {
            auto&[parent, child_it] = pair_noderef.value();
            auto side_it = _get_iter_by_direction(parent, child_it, _dir);
            if (side_it == parent.get().cend()) {
                if (parent.get().type() != window.parent()->get().get<Layout>().type()) {
                    window::purge_and_reconfigure(window);
                    switch (_dir) {
                    case Direction::Up:
                    case Direction::Left:
                        parent.get().insert(parent.get().begin(), window);
                        break;
                    case Direction::Down:
                    case Direction::Right:
                        parent.get().add(window);
                        break;
                    }
                    parent.get().update_rect();
                    return;
                } else return;
            };

            logger::debug("Move container -> can be moved to nearest neighbour");
            _move_to_nearest_neighbour(window, parent.get(), child_it, side_it, _dir);
        } else {
            Layout& top_layout = state.current_workspace().back()->get().get<Layout>();
            if (top_layout.size() == 1) return;

            switch (top_layout.type()) {
            case Layout::Type::Horizontal:
                if (_dir == Direction::Right || _dir == Direction::Left) return;
                break;
            case Layout::Type::Vertical:
                if (_dir == Direction::Up || _dir == Direction::Down) return;
                break;
            case Layout::Type::Tabbed:
                break;
            case Layout::Type::Floating: // impossible
                return;
            }

            logger::debug("Move container -> can be moved to top node");
            _move_to_top_node(window, state.current_workspace(), _dir);
        }
    }
#ifndef NDEBUG
    std::string str;
    _get_tree_numbers(str, state.current_workspace());
    logger::debug("Move container -> Tree: R{} {}", state.current_workspace().name(), str);
#endif
}

void Change_layout_type::operator()(State& state) const noexcept
{
    if (const auto& winref = state.current_workspace().window_list().current()) {
        logger::debug("Change layout type -> type: {}", layout_type_to_str(_change_to));

        Window& window = winref->get();
        window.mark_layout(_change_to);
    }
}