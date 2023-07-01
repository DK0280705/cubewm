#pragma once
#include "managed.h"
#include "container.h"

#include "helper/pointer_wrapper.h"
#include "helper/std_extension.h"

#include <utility>
#include <vector>
#include <ranges>

class Workspace;

class Monitor final : public Container
                    , public Managed<uint32_t>
{
    std::string             _name;
    Workspace*              _current;
    std::vector<Workspace*> _workspaces;

    void _update_rect_fn()  noexcept override;
    void _update_focus_fn() noexcept override;

public:
    HELPER_POINTER_ITERATOR_WRAPPER(_workspaces);

    inline auto size() const noexcept -> std::size_t
    { return _workspaces.size(); }

    inline auto empty() const noexcept -> bool
    { return _workspaces.empty(); }

    inline auto name() const noexcept -> std::string_view
    { return _name; }

    inline void current(const_iterator it)
    {
        assert(std::ranges::contains(_workspaces, &*it));
        _current = &*it;
    }

    inline auto current() const noexcept -> const Workspace&
    {
        assert(!empty());
        return (_current) ? *_current : *_workspaces.back();
    }

    inline auto current() noexcept -> Workspace&
    {
        assert(!empty());
        return (_current) ? *_current : *_workspaces.back();
    }

public:
     Monitor(const Index id, std::string name) noexcept
        : Managed(id)
        , _name(std::move(name))
        , _current(nullptr)
     {}

    void add(Workspace& workspace);
    void remove(const_iterator it);

    ~Monitor() noexcept override;
};
