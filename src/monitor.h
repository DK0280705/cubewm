#pragma once
#include "managed.h"
#include "node.h"
#include "container.h"

class Monitor : public Node<Container>
              , public Managed
{
public:
    Monitor(const Managed_id id) noexcept
        : Managed(id)
    {}

    void update_rect() override;

    ~Monitor() override
    { for (const auto& ws : *this) delete ws; }
};
