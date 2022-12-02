#pragma once
#include <xcb/xcb.h>
#include <xcb/xproto.h>

struct Rect
{
    int x;
    int y;
    int height;
    int width;
};

class Window
{
public:
    Window(xcb_connection_t* conn);

    xcb_window_t* prop;
    xcb_connection_t* conn;

    void set_focus(xcb_timestamp_t t);
    void set_rect(Rect r);
   
    xcb_atom_t get_window_type(xcb_get_property_type_t* reply);
   
    void add_atom(xcb_atom_t property, xcb_atom_t atom);
    void remove_atom(xcb_atom_t property, xcb_atom_t atom);
    
};
