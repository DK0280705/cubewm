#pragma once
#include <xcb/xproto.h>

class State;

namespace X11 {
class Connection;

struct Event
{
    xcb_generic_event_t* data;

    template <typename T>
    operator const T() const
    { return *(const T*)data; }
};

namespace event {
void init(const X11::Connection& conn);
void handle(State& state, const Event& event);
void ignore_unmap(xcb_window_t window_id);
bool is_unmap_ignored(xcb_window_t window_id);
}
}

