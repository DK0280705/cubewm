#pragma once
#include "container.h"
#include "manager.h"

class Window : public Container
             , public Focusable
             , public Managed
{
protected:
    std::string _name;
    Workspace*  _ws;

public:
    Window(Managed_id id) noexcept
        : Managed(id)
        , _ws(nullptr)
    {}

    inline std::string_view name() const
    { return _name; }

    inline Workspace* workspace() const
    { return _ws; }

    inline void workspace(Workspace* ws)
    { _ws = ws; }
};

// Have a good time with this :)
namespace X11 {

class Window : public ::Window
{
    using xcb_atom_t = unsigned int;
    xcb_atom_t  _type;
    std::string _role;
    std::string _class;
    std::string _instance;
    bool        _take_focus;

public:
    Window(Managed_id id);

    void update_rect(const Vector2D& rect) override;

    void focus() override;
    void unfocus() override;

private:
    void _fetch_name();
    void _fetch_type();
    void _fetch_role();
    void _fetch_class();
};

}


