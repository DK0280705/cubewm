#pragma once
#include "../connection.h"
#include <xcb/xproto.h>
/**
 * Separate X11 specifics from the actual window manager
 */

namespace X11 {
void init(const X11::Connection& conn);

// Namespace specific functions
// Now let's agree to not to use it outside namespace
// Discouraged for usage, use State::conn() instead.
// Only use if we desperately can't use State.
const Connection& _conn();
// This is root window id from the x11 screen struct.
unsigned int _root_window_id();
// This is a "cubewm" window we created under root window.
unsigned int _main_window_id();

namespace detail {
void check_error(const xcb_void_cookie_t& cookie);
}
}
