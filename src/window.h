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

class Window_list
{
    std::vector<Window*> _list;

public:
    inline auto current() const noexcept -> Window&
    { return *_list.back(); }

    inline bool empty() const noexcept
    { return _list.empty(); }

    HELPER_POINTER_ITERATOR_WRAPPER(_list);

    public:
    void add(Window& window);
    void focus(const_iterator it);
    void remove(const_iterator it);
};

class Window final : public Leaf<Container>
                   , public Managed<unsigned int>
{
public:
    enum class Display_type
    {
        X11,
        Wayland,
    };

    enum class State {
        Normal,
        Minimized,
        Maximized,
    };

    enum class Placement_mode
    {
        Tiling,
        Floating,
        Sticky, //soon
    };

    class Layout_mark
    {
    public:
        using Type = Layout::Containment_type;
    private:
        Type    _type;
        Window& _window;
        friend class Window;

        constexpr explicit Layout_mark(Window& window) noexcept
            : _type(Layout::Containment_type::Floating)
            , _window(window)
        {}
    public:
        inline auto window() const noexcept -> Window&
        { return _window; }
        inline auto type()   const noexcept -> Type
        { return _type; }
    };

    class Impl
    {
    protected:
        Impl() = default;

    public:
        virtual void update_rect()              noexcept = 0;
        virtual void update_focus()             noexcept = 0;
        virtual void update_state(Window::State) noexcept = 0;
        virtual ~Impl() noexcept = default;
    };

private:
    Display_type        _display_type;
    Window::State       _window_state;
    Placement_mode      _placement_mode;
    Layout_mark         _layout_mark;
    memory::owner<Impl> _impl;

    void _update_rect_fn()  noexcept override;
    void _update_focus_fn() noexcept override;

public:
    // Returns either X11 or Wayland
    inline auto display_type() const noexcept -> Display_type
    {
        return _display_type;
    }

    inline auto state() const noexcept -> Window::State
    {
        return _window_state;
    }

    inline auto placement_mode() const noexcept -> Placement_mode
    {
        return _placement_mode;
    }

    // Get layout mark
    inline auto layout_mark() -> std::optional<Layout_mark>
    {
        return is_marked() ? std::optional(_layout_mark) : std::nullopt;
    }
    // Mark window as layout
    inline void mark_layout(Layout_mark::Type lt) noexcept
    {
        assert(lt != Layout_mark::Type::Floating);
        _layout_mark._type = lt;
    }
    // Unmark window
    inline void unmark_layout() noexcept
    {
        _layout_mark._type = Layout_mark::Type::Floating;
    }

public:
    Window(Index id, Display_type dt);

    /**
     * @brief Check if window is marked as layout.
     * @return bool
     */
    bool is_marked() const noexcept;

    /**
     * @brief Set window to its normal state.
     * Show window if its minimized, or return window to its place from maximize.
     */
    void normalize() noexcept;
    /**
     * @brief Same as normalize(), but tiling.
     */
     void set_tiling() noexcept;
     /**
      * @brief Same as normalize(), but floating.
      */
     void set_floating() noexcept;

    /**
     * @brief Set window minimized.
     */
    void minimize() noexcept;

    /**
     * @brief Set window to fullscreen.
     * Yes, maximize here means fullscreen.
     */
    void maximize() noexcept;

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

// TODO: DELETE THIS
// Find a better way to hide implementation.
bool purge_sole_node(Node<Container> &node);
} // namespace window
