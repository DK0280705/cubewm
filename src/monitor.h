#pragma once
#include "managed.h"
#include "node.h"
#include "container.h"

class Monitor : public Node<Container>
              , public Managed<unsigned int>
{
public:
    Monitor(const Index id) noexcept
        : Managed(id)
    {}

    void update_rect() override;

    ~Monitor() override
    { for (const auto& ws : *this) delete ws; }
};
