#pragma once
#include "helper/pointer_wrapper.h"
#include "managed.h"
#include "container.h"
#include <vector>
#include <ranges>

class Workspace;

class Monitor : public Container
              , public Managed<unsigned int>
{
    std::string             _name;
    std::vector<Workspace*> _workspaces;

public:
    HELPER_POINTER_ITERATOR_WRAPPER(_workspaces);

    inline auto name() const noexcept -> std::string_view
    { return _name; }

    inline void add(Workspace& ws)
    { _workspaces.push_back(&ws); }

    inline void remove(Workspace& ws)
    { _workspaces.erase(std::ranges::find(_workspaces, &ws)); }

public:
    explicit Monitor(const Index id, const std::string& name) noexcept
        : Managed(id)
        , _name(name)
    {}

    void update_rect() noexcept override;

    virtual ~Monitor() noexcept;
};
