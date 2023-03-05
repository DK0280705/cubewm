#pragma once
#include "container.h"
#include "manager.h"
#include "visitor.h"
#include <unordered_set>

struct place;
class Window;

class Focus_list
{
    std::list<Window*>          _list;
    std::unordered_set<Window*> _pos;

public:
    inline Window* current() const
    { return _list.back(); }

    inline bool contains(Window* con) const
    { return _pos.contains(con); }

public:
    void add(Window* foc, bool focus = true);
    void remove(Window* foc);
};

class Workspace : public Node_box<Container>
                , public Managed
{
    Focus_list _focus_list;

public:
    VISITABLE_OVERRIDE(place);
    Workspace(const Managed_id id) noexcept
        : Managed(id)
    {}

    inline Focus_list& focus_list()
    { return _focus_list; }

    inline Layout_container* operator[](const int index) const
    { return dynamic_cast<Layout_container*>(*std::next(_children.begin(), index)); }

public:
    void update_rect() override;

    ~Workspace() override
    { for (const auto& c : _children) delete c; }
};

