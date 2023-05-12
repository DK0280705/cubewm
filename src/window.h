#pragma once
#include "container.h"
#include "geometry.h"
#include "node.h"
#include "frame.h"
#include "layout.h"
#include "managed.h"
#include <concepts>

class Workspace;
class Window_list;

class Window : public Leaf<Container>
             , public Focusable
             , public Managed<unsigned int>
{
protected:
    uint32_t                    _border_size{};
    Vector2D                    _frame_extents;
    std::string                 _name;
    Window_frame*               _frame;
    std::optional<Layout::Type> _layout_mark;

public:
    inline void name(const std::string& name) noexcept
    { _name = name; }
    inline std::string_view name() const noexcept
    { return _name; }

    inline Window_frame& frame() const noexcept
    { assert(_frame); return *_frame; }

    inline std::optional<Layout::Type> layout_mark() const noexcept
    { return _layout_mark; }

    inline void mark_layout(Layout::Type type) noexcept
    { _layout_mark = std::optional<Layout::Type>(type); }

    inline void unmark_layout() noexcept
    { _layout_mark = std::nullopt; }

public:
    Window(Index id)
        : Leaf<Container>()
        , Managed(id)
        , _layout_mark(std::nullopt)
    {}

    ~Window() noexcept
    {
        if (_frame) delete _frame;
    }
};

// Type safe container
struct Marked_window
{
    Window& window;

private:
    constexpr Marked_window(Window& window) noexcept : window(window) {}
    friend auto get_marked_window(Window& window) noexcept -> std::optional<Marked_window>;
};

bool is_window_marked(Window& window) noexcept;

auto get_marked_window(Window& window) noexcept -> std::optional<Marked_window>;

/**
 *  Move window to workspace.
 *  Creates a new top node if workspace empty.
 *  Suitable for mapping window.
*/
void move_to_workspace(Window& window, Workspace& workspace);

/**
 *  Move window to a marked window.
 *  Make sure window is layout marked or undefined behavior
*/
void move_to_marked_window(Window& window, Marked_window& m_window);

/**
 *  Purge window from its parents.
 *  This will purge parent if parent has only one child.
*/
void purge_and_reconfigure(Window& window);

bool purge_sole_node(Node<Container>& node);

std::optref<Window> find_window_by_position(Workspace& ws, const Point2D& pos);