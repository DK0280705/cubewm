#pragma once
#include "../state.h"

// Dekstops here means workspace

namespace X11::ewmh {
void update_active_window(unsigned int window);
void update_current_desktop(const State& state);
void update_client_list(const Manager<::Window>& mgr);
void update_number_of_desktops(const Manager<::Workspace>& mgr);
void update_desktop_names(const Manager<::Workspace>& mgr);
}