#pragma once
#include "container.h"
#include "geometry.h"
#include "node.h"
#include "layout.h"
#include "managed.h"

#include "helper/memory.h"

#include <concepts>
#include <vector>

class Window;
class Workspace;
class Window_frame;

class Window_list
{
    std::vector<Window*> _list;

    public:
    inline auto current() const noexcept -> std::optref<Window>
    { return _list.empty() ? std::nullopt : std::optref<Window>(*_list.back()); }

    inline bool empty() const noexcept
    { return _list.empty(); }

    HELPER_POINTER_ITERATOR_WRAPPER(_list);

    public:
    void add(Window& window);
    void focus(const_iterator it);
    void remove(const_iterator it);
};

class Window : public Leaf<Container>
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
        Layout::Type _type;
        bool         _marked;
        Window&      _window;
        friend class Window;

        constexpr explicit Layout_mark(Window& window) noexcept
            : _type(Layout::Type::Floating)
            , _marked(false)
            , _window(window) // any type
        {}
    public:
        inline auto window() const noexcept -> Window&
        { return _window; }
        inline auto type()   const noexcept -> Layout::Type
        { return _type; }
    };

    // defined in x11/window.h
    struct X11_property;

private:
    std::string                 _name;
    Display_type                _display_type;
    Layout_mark                 _layout_mark;
    memory::owner<Window_frame> _frame;
    memory::owner<X11_property> _xprop;

public:
    /**
     * Set window name
     * @param name
     */
    inline void name(const std::string& name) noexcept
    { _name = name; }

    /**
     * Get window name
     * @return
     */
    inline auto name() const noexcept -> std::string_view
    { return _name; }


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
    // Mark window as layout
    inline void mark_layout(Layout::Type lt) noexcept
    {
        _layout_mark._type   = lt;
        _layout_mark._marked = true;
    }
    // Unmark window
    inline void unmark_layout() noexcept
    {
        _layout_mark._marked = false;
    }

    // Returns window frame
    inline auto frame() const noexcept -> Window_frame&
    {
        return *_frame;
    }

    inline auto xprop() const noexcept -> X11_property&
    {
        assert(_display_type == Display_type::X11);
        return *_xprop;
    }

protected:
    void _fill_xprop(memory::owner<X11_property> xprop) noexcept;

    void _fill_frame(memory::owner<Window_frame> frame) noexcept;

    Window(Index id, Display_type dt) noexcept;

public:
    /**
     * @brief Check if window is marked as layout.
     * @return bool
     */
    bool is_marked() const noexcept;

    ~Window() noexcept override;
};

namespace window {
/**
 * @brief Add window to window list.
 * Performs additional debug checks
 * @param window_list
 * @param window
 */
void add_window(Window_list &window_list, Window &window);

/**
 * @brief Focus a window from window list.
 * Performs additional debug checks
 * @param window_list
 * @param window
 */
void focus_window(Window_list &window_list, Window &window);

/**
 * @brief Focus last window in a window list.
 * Do nothing if it's empty.
 * @param window_list
 */
void focus_last(Window_list &window_list);

/**
 * @brief Try to focus a window from its window list.
 * Performs additional debug checks
 * @param window
 */
void try_focus_window(Window &window);

/**
 * @brief Remove window from window list.
 * Performs additional debug checks
 * @param window_list
 * @param window
 */
void remove_window(Window_list &window_list, Window &window);


/**
 * @brief Move window to workspace.
 *
 * @param window
 * @param workspace
 */
void move_to_workspace(Window &window, Workspace &workspace);

/**
 * @brief Move window to a new container with a marked window.
 * Creates a new layout from window layout mark type.
 * @param window
 * @param layout_mark Layout mark from window.layout_mark()
 */
void move_to_marked_window(Window &window, Window::Layout_mark &layout_mark);


/**
 * @brief Purge window from its parents.
 * Could remove its parent if the parent is empty.
 * @param window
 */
void purge_and_reconfigure(Window &window);

bool purge_sole_node(Node<Container> &node);
} // namespace window