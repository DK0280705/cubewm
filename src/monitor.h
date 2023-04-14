#pragma once
#include "helper.h"
#include "managed.h"
#include "workspace.h"

class Monitor : public Container
              , public Managed<unsigned int>
{
    std::vector<Workspace*> _workspaces;

public:
    DEFINE_POINTER_ITERATOR_WRAPPER(_workspaces);

    inline void add(Workspace& ws)
    { _workspaces.push_back(&ws); }

    inline void remove(Workspace& ws)
    { _workspaces.erase(std::ranges::find(_workspaces, &ws)); }

public:
    Monitor(const Index id) noexcept
        : Managed(id)
    {}

    void update_rect() noexcept override;

    virtual ~Monitor()
    { for (const auto& ws : *this) delete &ws; }
};
