#pragma once

// Dekstops here means workspace

class State;

namespace X11::ewmh {
void update_number_of_desktops(State& state);
void update_current_desktop(State& state);
void update_desktop_names(State& state);
void update_active_window(State& state, unsigned int window);
void update_client_list(State& state);
}