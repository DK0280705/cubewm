#pragma once
#include <string>
#include <vector>
extern "C" {
#include <xcb/xcb.h>
#include <xcb/xproto.h>
}

class Server;

class Win // Avoid name collision
{
    Server* _srv;
public:
    Win(Server* srv);

    xcb_window_t id;

    bool vertical;
    bool floating;
    std::string name;
    std::string role;

    void set_focus(xcb_timestamp_t t);
   
    xcb_atom_t get_window_type(xcb_get_property_type_t* reply);
    
    void add_atom(xcb_atom_t property, xcb_atom_t atom);
    void remove_atom(xcb_atom_t property, xcb_atom_t atom);
};
