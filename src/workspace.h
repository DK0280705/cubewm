#pragma once
#include "container.h"
#include "manager.h"
#include "visitor.h"
#include <unordered_set>

struct place;

class Focus_list
{
    std::list<Focusable*>          _list;
    std::unordered_set<Focusable*> _pos;

public:
    inline Focusable* current() const
    { return _list.back(); }

    inline bool contains(Focusable* con) const
    { return _pos.contains(con); }

public:
    void add(Focusable* foc, bool focus = true);
    void remove(Focusable* foc);
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
    void update_rect(const Vector2D& rect) override;

    ~Workspace() override
    { for (const auto& c : _children) delete c; }
};

