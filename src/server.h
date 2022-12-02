#pragma once
extern "C" {
#include <xcb/xcb.h>
#include <xcb/xproto.h>
}

class Server
{
    void _get_timestamp();
    void _get_wm_sn_atom();
public:
    Server();
    Server(const Server& s) = delete;
    Server(const Server&& s) = delete;

    ~Server();

    void run();

    int screen_id;

    xcb_connection_t* conn;
    xcb_timestamp_t timestamp;
    xcb_atom_t wm_sn;
    xcb_window_t wm_sn_owner;
    
    xcb_screen_t *root_screen;

    xcb_window_t create_window();
};
