#pragma once
#include "../helper.h"
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

class Event_handler_impl;
class Event_handler : public Init_once<Event_handler>
{
    Event_handler_impl& _impl;

public:
    Event_handler(State& state);

    void handle(const Event& event);
};

}
