#pragma once
#include "layout.h"
#include "helper.h"
#include "managed.h"

struct place;
class Window;

class Window_list
{
    std::vector<Window*> _list;

public:
    inline std::optref<Window> last() const noexcept
    { return _list.empty() ? std::nullopt : std::optref<Window>(*_list.back()); }

    inline bool empty() const noexcept
    { return _list.empty(); }

    DEFINE_POINTER_ITERATOR_WRAPPER(_list);

public:
    void add(Window& window);
    void focus(const_iterator it);
    void remove(const_iterator it);
};

class Workspace : public Root<Container>
                , public Managed<unsigned int>
{
    std::string _name;
    Window_list _window_list;

    Layout* _focused_layout;

public:
    Workspace(const Index id);

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

public:
    void update_rect() noexcept override;

    ~Workspace() noexcept override
    {
        for (const auto& c : *this) delete &c;
    }
};

/**
 * @brief Add window to window list.
 * Performs additional debug checks
 * @param window_list
 * @param window
 */
void add_window(Window_list& window_list, Window& window);

/**
 * @brief Focus a window from window list.
 * Performs additional debug checks
 * @param window_list
 * @param window
 */
void focus_window(Window_list& window_list, Window& window);

/**
 * @brief Focus last window in a window list.
 * Do nothing if it's empty.
 * @param window_list
 */
void focus_last(Window_list& window_list);

/**
 * @brief Try to focus a window from its window list.
 * Performs additional debug checks
 * @param window
 */
void try_focus_window(Window& window);

/**
 * @brief Remove window from window list.
 * Performs additional debug checks
 * @param window_list
 * @param window
 */
void remove_window(Window_list& window_list, Window& window);
