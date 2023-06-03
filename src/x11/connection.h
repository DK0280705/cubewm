#pragma once

struct xcb_connection_t;
struct xcb_screen_t;

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