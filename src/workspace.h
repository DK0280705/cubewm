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
    Window_list _window_list;
    std::string _name;

    // Exclusive floating node
    Layout* _floating_node;
    Layout* _focused_layout;

public:
    Workspace(const Index id) noexcept;

    inline Window_list& window_list() noexcept
    { return _window_list; }

    inline const Window_list& window_list() const noexcept
    { return _window_list; }

    inline std::string_view name() const noexcept
    { return _name; }

    inline Layout& floating_node() const noexcept
    { return *_floating_node; }

    inline std::optref<Layout> focused_layout() const noexcept
    { return _focused_layout ? std::optref<Layout>(*_focused_layout) : std::nullopt; }

    inline void focused_layout(Layout& layout) noexcept
    { _focused_layout = &layout; }

public:
    void update_rect() noexcept override;

    ~Workspace() noexcept override
    {
        for (const auto& c : *this) delete &c;
        delete _floating_node;
    }
};
void add_window(Window_list& window_list, Window& window, bool focus = false);
void focus_window(Window_list& window_list, Window& window);
void focus_last(Window_list& window_list);
void remove_window(Window_list& window_list, Window& window);