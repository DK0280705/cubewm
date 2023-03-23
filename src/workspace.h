#pragma once
#include "container.h"
#include "helper.h"
#include "managed.h"
#include <unordered_set>

struct place;
class Window;

class Window_list
{
    std::list<Window*>          _list;
    std::unordered_set<Window*> _pos;

public:
    using Iterator       = typename decltype(_list)::iterator;
    using Const_iterator = typename decltype(_list)::const_iterator;

public:
    inline Window* current() const
    { return _list.back(); }

    inline bool contains(Window* con) const
    { return _pos.contains(con); }

    DECLARE_ITERATOR_WRAPPER(_list)

public:
    void add(Window* foc);
    void focus(Const_iterator it);
    void remove(Window* foc);
};

class Workspace : public Node<Container>
                , public Managed
{
    Window_list _window_list;

public:
    Workspace(const Managed_id id) noexcept
        : Managed(id)
    {}

    inline Window_list& window_list()
    { return _window_list; }

public:
    void update_rect() override;

    ~Workspace() override
    { for (const auto& c : *this) delete c; }
};

