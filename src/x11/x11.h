#pragma once
/**
 * Separate X11 specifics from the actual window manager
 */

class State;

namespace X11 {
class Connection; // Ofcourse, this refer to X11::Connection

void init(::State& state);

// Namespace specific functions
// Now let's agree to not to use it outside namespace
Connection& _conn();
unsigned int _main_window_id();
}
