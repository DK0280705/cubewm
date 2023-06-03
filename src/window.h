#pragma once
#include "container.h"
#include "geometry.h"
#include "helper.h"
#include "node.h"
#include "frame.h"
#include "layout.h"
#include "managed.h"
#include <concepts>
#include <cstddef>

class Workspace;
class Window_list;

class Window : public Leaf<Container>
             , public Focusable
             , public Managed<unsigned int>
{
public:
    enum class Display_type
    {
        X11,
        Wayland,
    };

    class Layout_mark
    {
        Window&      _window;
        bool         _marked;
        Layout::Type _type;
        friend class Window;

        constexpr Layout_mark(Window& window) noexcept
            : _window(window)
            , _marked(false)
            , _type(Layout::Type::Floating) // any type
        {}
    public:
        inline auto window() const noexcept -> Window&
        { return _window; }
        inline auto type() const noexcept -> Layout::Type
        { return _type; }
    };

    // defined in x11/window.h
    struct X11_property;

private:
    std::string   _name;
    Vector2D      _actual_size;
    Display_type  _display_type;
    Layout_mark   _layout_mark;
    Window_frame* _frame;

public:
    // Set/Get window name
    inline void name(const std::string& name) noexcept
    {
        _name = name;
    }
    inline auto name() const noexcept -> std::string_view
    {
        return _name;
    }

    // Returns window's actual size
    inline auto actual_size() const noexcept -> const Vector2D&
    {
        return _actual_size;
    }

    // Returns either X11 or Wayland
    inline auto display_type() const noexcept -> Display_type
    {
        return _display_type;
    }

    // Get layout mark
    inline auto layout_mark() -> std::optional<Layout_mark>
    {
        return is_marked() ? std::optional(_layout_mark) : std::nullopt;
    }
    // Layout mark modifier
    inline void mark_layout(Layout::Type lt) noexcept
    {
        _layout_mark._type   = lt;
        _layout_mark._marked = true;
    }
    inline void unmark_layout() noexcept
    {
        _layout_mark._marked = false;
    }

    // Returns window frame
    inline auto frame() const noexcept -> Window_frame&
    {
        return *_frame;
    }

    inline auto xprop() noexcept -> X11_property&
    {
        assert(_display_type == Display_type::X11);
        return *_xprop;
    }

protected:
    X11_property* _xprop;

    Window(Index id, Display_type dt, Window_frame* frame) noexcept
        : Leaf<Container>()
        , Managed(id)
        , _display_type(dt)
        , _layout_mark(Layout_mark(*this))
        , _frame(frame)
    {}

    virtual void _update_rect() noexcept = 0;

public:
    bool is_marked() const noexcept;

    void update_rect() noexcept override;

    ~Window() noexcept
    {
        if (_frame) delete _frame;
    }
};


/**
 * @brief Move window to workspace.
 *
 * @param window
 * @param workspace
 */
void move_to_workspace(Window& window, Workspace& workspace);

/**
 * @brief Move window to a new container with a marked window.
 * Creates a new layout from window layout mark type.
 * @param window
 * @param layout_mark Layout mark from window.layout_mark()
 */
void move_to_marked_window(Window& window, Window::Layout_mark& layout_mark);


/**
 * @brief Purge window from its parents.
 * Could remove its parent if the parent is empty.
 * @param window
 */
void purge_and_reconfigure(Window& window);

bool purge_sole_node(Node<Container>& node);