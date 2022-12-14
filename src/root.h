#pragma once
#include "container.h"
#include <xcb/xproto.h>

class Root : public Container
{
    Root(Server* srv);
    bool _get_monitors();
    bool _randr_init();
public:
    ~Root();

    static Root* init(Server* srv); // Well, there's only one root.

    Output& create_output(const std::string& name, struct Rect rect);

    Win& get_window_by_id(xcb_window_t id);
};
