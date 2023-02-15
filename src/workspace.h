#pragma once
#include "container.h"
#include "manager.h"

class Workspace : public Node_box<Container>
                , public Visit<Workspace>
                , public Managed
{
    Container* _focused;

public:
    Workspace(const Managed_id id) noexcept
        : Managed(id)
    {}

    inline Container* focused() const
    { return _focused; }

    inline void focused(Container* con)
    { _focused = con; }

    inline Layout_container* operator[](const int index) const
    { return dynamic_cast<Layout_container*>(*std::next(_children.begin(), index)); }

public:
    void update_rect(const Vector2D& rect) override;

    ~Workspace() override
    { for (const auto& c : _children) delete c; }
};

