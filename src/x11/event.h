#pragma once
#include <xcb/xproto.h>

class State;

namespace X11 {

struct Event
{
    xcb_generic_event_t* data;
    
    template <typename T>
    operator const T() const
    { return *(const T*)data; }
};

namespace event {
void init(State& state);
void handle(const Event& event);
}
}