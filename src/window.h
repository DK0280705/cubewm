#pragma once
#include "container.h"
#include "frame.h"
#include "managed.h"
#include <concepts>

class Window_frame;
class Workspace;

class Window : public Leaf<Container>
             , public Focusable
             , public Managed<unsigned int>
{
protected:
    bool          _busy;
    std::string   _name;
    Window_frame* _frame;

public:
    Window(Index id, Window_frame* frame)
        : Managed(id)
        , _busy(false)
        , _frame(frame)
    {}

    inline std::string_view name() const noexcept
    { return _name; }

    inline Window_frame& frame() const noexcept
    { return *_frame; }

    inline bool busy() const noexcept
    { return _busy; }

    inline void busy(const bool b) noexcept
    { _busy = b; }

    void accept(const container_visitor& visitor) noexcept override
    { visitor(*this); }

    ~Window() noexcept
    { delete _frame; }
};

void place_to(Workspace& ws, Window& window, bool create_new = false);
void purge(Window& window);
std::optref<Window> find_window_by_position(Workspace& ws, const Point2D& pos);