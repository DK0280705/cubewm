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

    inline bool empty() const noexcept
    { return _workspaces.empty(); }

    inline auto name() const noexcept -> std::string_view
    { return _name; }

    void add(Workspace& workspace);

    void remove(const_iterator it);

    inline void current(const_iterator it)
    {
        assert(std::ranges::contains(_workspaces, &*it));
        _current = &*it;
    }

    inline auto current() const noexcept -> std::optref<Workspace>
    {
        return _current ? std::optref<Workspace>(*_current)
                        : std::nullopt;
    }

public:
     Monitor(const Index id, std::string name) noexcept
        : Managed(id)
        , _name(std::move(name))
        , _current(nullptr)
     {}

    ~Monitor() noexcept override;
};
