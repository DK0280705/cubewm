#pragma once
#include "connection.h"
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
auto conn() noexcept -> const Connection&;
// This is root window id from the x11 screen struct.
auto root_window_id() noexcept -> unsigned int;
// This is a "cubewm" window we created under root window.
auto main_window_id() noexcept -> unsigned int;
// Check for void cookie error return, if exists throws.
void check_error(const xcb_void_cookie_t& cookie);
}
}
