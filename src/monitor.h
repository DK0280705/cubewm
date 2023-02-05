#pragma once
#include "manager.h"
#include "node.h"
#include "container.h"

class Monitor : public Node<Container>
              , public Managed
{
public:
    Monitor(const Managed_id id) noexcept
        : Managed(id)
    {}

    void update_focus()                    override;
    void update_rect(const Vector2D& rect) override;
};
