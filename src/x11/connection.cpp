#include "../connection.h"
#include "../helper.h"
#include "x11.h"
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

namespace X11 {

Connection::Connection()
    : _scr_id(0)
    , _conn(xcb_connect(nullptr, &_scr_id))
    , _screen(xcb_aux_get_screen(_conn, _scr_id))
{
    assert_runtime(!xcb_connection_has_error(_conn), "Failed to start connection");
    assert_runtime(_screen, "Failed to get display screen");
    // Init X11 first, so we can call the xcb functions.
    X11::init(*this);
}

Connection::~Connection()
{
    xcb_disconnect(_conn);
}

}
