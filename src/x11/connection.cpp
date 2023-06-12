#include "connection.h"
#include "../error.h"
#include "../helper/memory.h"
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_keysyms.h>

namespace X11 {

Connection::Connection()
    : _scr_id(0)
    , _conn(xcb_connect(nullptr, &_scr_id))
    , _screen(xcb_aux_get_screen(_conn, _scr_id))
    , _keysyms(xcb_key_symbols_alloc(_conn))
{
    assert_runtime<Connection_error>(!xcb_connection_has_error(_conn), "Failed to start connection");
    assert_runtime<Connection_error>(_screen, "Failed to get display screen");
}

void Connection::flush() const noexcept
{
    xcb_flush(_conn);
}

void Connection::sync() const noexcept
{
    xcb_aux_sync(_conn);
}

Connection::~Connection()
{
    xcb_key_symbols_free(_keysyms);
    xcb_disconnect(_conn);
}

auto root_window_id(const X11::Connection& conn) noexcept -> xcb_window_t
{
    return conn.xscreen()->root;
}

auto keysym_to_keycode(const X11::Connection& conn, xcb_keysym_t keysym) noexcept -> xcb_keycode_t
{
    auto ptr = memory::c_own(xcb_key_symbols_get_keycode(conn.keysyms(), keysym));
    return *ptr;
}

} // namespace X11
