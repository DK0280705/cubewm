#pragma once
#include "layout.h"
#include "managed.h"
#include "helper/pointer_wrapper.h"

class Window;
class Monitor;

class Workspace final : public Root<Container>
                      , public Managed<unsigned int>
{
    class _Window_list
    {
        std::list<Window*> _list;

    public:
        HELPER_POINTER_ITERATOR_WRAPPER(_list);

        inline auto current() const noexcept -> Window&
        { return *_list.front(); }

        inline bool empty() const noexcept
        { return _list.empty(); }

        public:
        void add(Window& window)       noexcept;
        void focus(const_iterator it)  noexcept;
        void remove(const_iterator it) noexcept;
    };

    friend class Monitor;
    Monitor*     _monitor;
    std::string  _name;
    _Window_list _window_list;

    void _update_rect_fn()  noexcept override;
    void _update_focus_fn() noexcept override;

public:
    explicit Workspace(Index id);

    inline auto monitor() const noexcept -> Monitor&
    { assert(_monitor); return *_monitor; }

    inline auto name() const noexcept -> std::string_view
    { return _name; }

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

    inline bool has_window() const noexcept
    { return !_window_list.empty(); }

    inline auto current_window() const noexcept -> Window&
    { return _window_list.current(); }

public:

    void add_window(Window& window)    noexcept;
    void focus_window(Window& window)  noexcept;
    void remove_window(Window& window) noexcept;

    ~Workspace() noexcept override;
};