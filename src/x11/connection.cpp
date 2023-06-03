#include "../connection.h"
#include "../helper.h"
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

namespace X11 {

Connection::Connection()
    : _scr_id(0)
    , _conn(xcb_connect(nullptr, &_scr_id))
    , _screen(xcb_aux_get_screen(_conn, _scr_id))
{
    assert_runtime<Connection_error>(!xcb_connection_has_error(_conn), "Failed to start connection");
    assert_runtime<Connection_error>(_screen, "Failed to get display screen");
}

Connection::~Connection()
{
    xcb_disconnect(_conn);
}

} // namespace X11

uint32_t root_window_id(const X11::Connection& conn) noexcept
{
    return conn.xscreen()->root;
}