#pragma once
#include "container.h"
#include "node.h"
#include "frame.h"
#include "layout.h"
#include "managed.h"
#include <concepts>

class Workspace;

class Window : public Leaf<Container>
             , public Focusable
             , public Managed<unsigned int>
{
protected:
    bool          _busy;
    std::string   _name;
    Window_frame* _frame;

    std::optional<Layout::Type> _marked_layout_type;

public:
    Window(Index id, Window_frame* frame)
        : Leaf<Container>()
        , Managed(id)
        , _busy(false)
        , _frame(frame)
        , _marked_layout_type(std::nullopt)
    {}

    inline std::string_view name() const noexcept
    { return _name; }

    inline Window_frame& frame() const noexcept
    { return *_frame; }

    inline bool busy() const noexcept
    { return _busy; }

    inline void busy(const bool b) noexcept
    { _busy = b; }

    inline std::optional<Layout::Type> layout_mark() const noexcept
    { return _marked_layout_type; }

    inline void mark_as_layout(Layout::Type type) noexcept
    { _marked_layout_type = std::optional<Layout::Type>(type); }

    inline void unmark_layout() noexcept
    { _marked_layout_type = std::nullopt; }

    void accept(const container_visitor& visitor) noexcept override
    { visitor(*this); }

    ~Window() noexcept
    { if (_frame) delete _frame; }
};

void place(Window& window, Workspace& workspace);
// return false if window is not marked.
bool move_to_marked_window(Window& window, Window& marked_window);
void purge(Window& window, bool rebase = true);
bool purge_sole_node(Node<Container>& node);
std::optref<Window> find_window_by_position(Workspace& ws, const Point2D& pos);