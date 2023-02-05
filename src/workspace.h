#pragma once
#include "container.h"
#include "manager.h"

class Workspace : public Node<Container>
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

public:
    void update_focus()                    override;
    void update_rect(const Vector2D& rect) override;

    ~Workspace() override
    { for (const auto& c : _children) delete c; }
};

