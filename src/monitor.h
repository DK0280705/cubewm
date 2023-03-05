#pragma once
#include "manager.h"
#include "node.h"
#include "container.h"

class Monitor : public Node_box<Container>
              , public Managed
{
public:
    Monitor(const Managed_id id) noexcept
        : Managed(id)
    {}

    void update_rect() override;
};
