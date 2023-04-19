#include "binding.h"
#include "geometry.h"
#include "layout.h"
#include "logger.h"
#include "state.h"

static void _focus(Window& window) noexcept
{
    assert(window.is_leaf());

    auto& window_list = window.root<Workspace>().window_list();
    window_list.focus(std::ranges::find(window_list, window));
}

static void _focus_front_leaf(Node<Container>& node) noexcept
{
    Node<Container>* leaf = &node;
    while (!leaf->is_leaf()) leaf = &leaf->front()->get();
    _focus(leaf->leaf<Window>());
}

template<Direction prev, Direction next> requires(prev != next)
static auto _get_iter_by_direction(Node<Container>& parent, Node<Container>& current, Direction dir)
    -> Node<Container>::const_iterator
{
    auto current_it = std::ranges::find(parent, current);
    switch(dir) {
        case prev: return std::ranges::prev(current_it);
        case next: return std::ranges::next(current_it);
        default: return parent.end();
    }
}

static auto _get_iter_by_direction(Layout& parent, Node<Container>& current, Direction dir)
    -> Node<Container>::const_iterator
{
    if (parent.size() == 1) return parent.cend();
    switch (parent.type()) {
    case Layout::Type::Horizontal:
        return _get_iter_by_direction<Direction::Left, Direction::Right>(parent, current, dir);
    case Layout::Type::Vertical:
        return _get_iter_by_direction<Direction::Up, Direction::Down>(parent, current, dir);
    case Layout::Type::Tabbed:
        return _get_iter_by_direction<Direction::Left, Direction::Right>(parent, current, dir);
    default:
        return parent.cend();
    }
}

void Move_focus::operator()(State& state) const noexcept
{
    if (const auto& winref = state.current_workspace().window_list().current()) {
        logger::debug("Move focus -> direction: {}", direction_to_str(_dir));

        Window& window = winref->get();

        Node<Container>* current = &window;
        Node<Container>* result  = nullptr;

        int i = 0;
        while(true) {
            logger::debug("Move focus -> iteration: {}", ++i);
            assert(current->parent());
            Layout& parent = static_cast<Layout&>(current->parent()->get());

            auto it = _get_iter_by_direction(parent, *current, _dir);

            if (it == parent.end()) {
                assert(parent.parent());
                if (!parent.parent()->get().is_root()) {
                    current = &parent;
                    continue;
                } else break;
            }

            result = &*it;
            break;
        }

        if (result) _focus_front_leaf(*result);
    }
}

void Move_container::operator()(State& state) const noexcept
{
    if (const auto& winref = state.current_workspace().window_list().current()) {
        logger::debug("Move focus -> direction: {}", direction_to_str(_dir));

        Window& window = winref->get();

        assert(window.parent());
    }
}

void Change_layout_type::operator()(State& state) const noexcept
{
    if (const auto& winref = state.current_workspace().window_list().current()) {
        logger::debug("Change layout type -> type: {}", layout_type_to_str(_change_to));

        Window& window = winref->get();
        window.mark_as_new_layout(_change_to);
    }
}