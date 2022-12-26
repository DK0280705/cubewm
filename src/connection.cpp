#include "connection.h"
#include "error.h"
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

Timestamp::Timestamp()
    : _timestamp(XCB_CURRENT_TIME)
{}

Connection::Connection()
    : timestamp()
    , _scr_id(0)
    , _conn(xcb_connect(nullptr, &_scr_id))
    , _screen(xcb_aux_get_screen(_conn, _scr_id))
{
    assert_runtime(!xcb_connection_has_error(_conn), "Cannot open connection!");
}

Connection& Connection::init()
{
    static Connection conn;
    assert_init(conn);
    return conn;
}

Connection::~Connection()
{
    xcb_disconnect(_conn);
}
