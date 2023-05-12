#pragma once
#include "../connection.h"
#include <xcb/xproto.h>
/**
 * Separate X11 specifics from the actual window manager
 */

namespace X11 {
void init(const X11::Connection& conn);

namespace detail {
// Namespace specific functions
// Now let's agree to not to use it outside namespace
// Discouraged for usage, use State::conn() instead.
// Only use if we desperately can't use State.
const Connection& conn() noexcept;
// This is root window id from the x11 screen struct.
unsigned int root_window_id() noexcept;
// This is a "cubewm" window we created under root window.
unsigned int main_window_id() noexcept;
// Check for void cookie error return, if exists throws.
void check_error(const xcb_void_cookie_t& cookie);
}
}
