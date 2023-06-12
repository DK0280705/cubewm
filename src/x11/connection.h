#pragma once
#include <cstdint>

struct xcb_connection_t;
struct xcb_screen_t;

using xcb_window_t      = uint32_t;
using xcb_keysym_t      = uint32_t;
using xcb_keycode_t     = uint8_t;
using xcb_key_symbols_t = struct _XCBKeySymbols;

namespace X11 {
class Connection
{
    int                _scr_id;
    xcb_connection_t*  _conn;
    xcb_screen_t*      _screen;
    xcb_key_symbols_t* _keysyms;

protected:
    Connection();

public:
    inline operator xcb_connection_t* () const noexcept
    { return _conn; }

    auto xscreen() const noexcept -> xcb_screen_t*
    { return _screen; }

    auto scr_id()  const noexcept -> int
    { return _scr_id; }

    auto keysyms() const noexcept -> xcb_key_symbols_t*
    { return _keysyms; }

    void flush()   const noexcept;

    void sync()    const noexcept;

    virtual ~Connection();
};

auto root_window_id(const X11::Connection& conn) noexcept -> xcb_window_t;

auto keysym_to_keycode(const X11::Connection& conn, xcb_keysym_t keysym) noexcept -> xcb_keycode_t;

} // namespace X11
