#pragma once
#include "container.h"
#include "managed.h"

class Window_frame;

class Window : public Node<Container>
             , public Managed
{
protected:
    std::string _name;
    Workspace*  _ws;
    bool        _focused;
    bool        _busy;

    std::unique_ptr<Window_frame> _frame;

public:
    Window(Managed_id id) noexcept
        : Managed(id)
        , _ws(nullptr)
        , _focused(false)
        , _busy(false)
    {}

    inline std::string_view name() const noexcept
    { return _name; }

    inline Workspace* workspace() const noexcept
    { return _ws; }

    inline void workspace(Workspace* ws) noexcept
    { _ws = ws; }

    inline Window_frame* frame() const noexcept
    { return _frame.get(); }

    inline bool focused() const noexcept
    { return _focused; }

    inline bool busy() const noexcept
    { return _busy; }
    
    inline void busy(const bool b) noexcept
    { _busy = b; }


public:
    virtual void focus() = 0;
    virtual void unfocus() = 0;
};

class Window_frame : public Managed
{
    Window* _window;
public:
    Window_frame(Managed_id id, Window* window) noexcept
        : Managed(id)
        , _window(window)
    {}

    inline Window* window() const noexcept
    { return _window; }
};

void place_to(Workspace* ws, Window* window, bool create_new = false);
void purge(Window* window);
Window* find_window_by_position(Workspace* ws, const Point2D& pos);