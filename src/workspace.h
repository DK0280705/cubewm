#pragma once
#include "layout.h"
#include "window.h"
#include "helper/pointer_wrapper.h"

class Monitor;

class Workspace final : public Root<Container>
                      , public Managed<unsigned int>
{
    std::string _name;
    Window_list _window_list;
    Layout*     _focused_layout;
    Monitor*    _monitor;
    friend class Monitor;

    void _update_rect_fn()  noexcept override;
    void _update_focus_fn() noexcept override;

public:
    explicit Workspace(Index id);

    inline auto name() const noexcept -> std::string_view
    { return _name; }

    inline auto window_list() noexcept -> Window_list&
    { return _window_list; }
    inline auto window_list() const noexcept -> const Window_list&
    { return _window_list; }

    inline auto focused_layout() const noexcept -> std::optref<Layout>
    { return _focused_layout ? std::optref<Layout>(*_focused_layout) : std::nullopt; }
    inline void focused_layout(Layout& layout) noexcept
    { _focused_layout = &layout; }

    inline auto tiling_layout() const -> Layout&
    {
        if (!empty() || size() == 2) {
            return back().get<Layout>();
        } else throw Existence_error("Tiling layout not exist");
    }

    inline auto floating_layout() const -> Layout&
    {
        // Floating layout is initialised at constructor
        assert(!empty());
        return front().get<Layout>();
    }

    inline auto monitor() const noexcept -> Monitor&
    { assert(_monitor); return *_monitor; }

    ~Workspace() noexcept override;
};
