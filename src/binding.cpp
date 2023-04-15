#include "binding.h"
#include "container.h"
#include "logger.h"
#include "state.h"

struct move_focus_visit : public container_visitor
{
private:
    const Node<Container>&        _con;
    const Direction               _dir;

private:
    void _focus(Window& result) const noexcept
    {
        logger::debug("Move focus -> has value");
        assert(result.is_leaf());

        auto& window_list = result.root<Workspace>().window_list();
        window_list.focus(std::ranges::find(window_list, result));
    }

    template<Direction prev, Direction next>
    requires(prev != next)
    void _get_direction_impl(Node<Container>& parent) const noexcept
    {
        assert(parent == _con.parent());
        if (parent.size() == 1) return;

        auto iter = std::ranges::find(parent, _con);

        switch (_dir) {
        case prev:
        {
            auto prev_it = std::ranges::prev(iter);
            if (prev_it == parent.end())
                prev_it = std::ranges::prev(parent.end());

            Node<Container>* leaf = &*prev_it;
            while (!leaf->is_leaf()) leaf = &leaf->front()->get();
            _focus(leaf->leaf<Window>());

            break;
        }
        case next:
        {
            auto next_it = std::ranges::next(iter);
            if (next_it == parent.end())
                next_it = parent.begin();

            Node<Container>* leaf = &*next_it;
            while (!leaf->is_leaf()) leaf = &leaf->front()->get();
            _focus(leaf->leaf<Window>());
        }
        default:
            break;
        }
    }

public:
    move_focus_visit(const Node<Container>& con, Direction dir) noexcept
        : _con(con)
        , _dir(dir)
    {}

    void operator()(class Monitor&) const noexcept override {};
    void operator()(class Workspace&) const noexcept override {};
    void operator()(class Horizontal_container& hcon) const noexcept override
    {
        _get_direction_impl<Direction::left, Direction::right>(hcon);
    };
    void operator()(class Vertical_container& vcon) const noexcept override
    {
        _get_direction_impl<Direction::up, Direction::down>(vcon);
    };
    void operator()(class Tabbed_container& tcon) const noexcept override
    {
        _get_direction_impl<Direction::left, Direction::right>(tcon);
    };
    void operator()(class Window&) const noexcept override
    {};
};

void Move_focus::operator()(State& state) const
{
    if (const auto& conref = state.current_workspace().window_list().current()) {
        logger::debug("Move focus -> direction: {}", (int)_dir);

        Node<Container>& con = conref->get();

        assert(con.parent());
        con.parent()->get().accept(move_focus_visit(con, _dir));
    }
}