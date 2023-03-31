#pragma once
/**
 * Define X11 server connection
 * Maybe extended with Wayland compositor
 * I'll give it a try
 */
#include "helper.h"

struct xcb_connection_t;
struct xcb_screen_t;

struct wl_display;
struct wl_surface;
struct wl_shell;

namespace X11 {
class Connection
{
    int               _scr_id;
    xcb_connection_t* _conn;
    xcb_screen_t*     _screen;

protected:
    Connection();

public:

    operator xcb_connection_t* () const
    { return _conn; }

    const xcb_screen_t* xscreen() const
    { return _screen; }

    int scr_id() const
    { return _scr_id; }

    virtual ~Connection();
};
} // namespace X11

namespace Wayland {
class Connection
{
    wl_display* _dpy;
    wl_surface* _sfc;
    wl_shell*   _sh;

protected:
    Connection();

public:

    operator wl_display* () const
    { return _dpy; }

    const wl_surface* surface() const
    { return _sfc; }

    const wl_shell* shell() const
    { return _sh; }

    virtual ~Connection();
};
} // namespace Wayland

class Connection : public X11::Connection
#ifdef USE_WAYLAND
                 , public Wayland::Connection
#endif
                 , public Init_once<Connection>
{
public:
    Connection()
        : X11::Connection()
#ifdef USE_WAYLAND
        , Wayland::Connection()
#endif
    {}
};
