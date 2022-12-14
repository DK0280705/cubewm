#pragma once

extern "C" {
#include <xcb/xproto.h>
}

class Server;

class EWMH
{
    Server* _srv;
    EWMH(Server* srv);

public:
    EWMH(const EWMH& e)  = delete;
    EWMH(const EWMH&& e) = delete;

    ~EWMH();
    
    static EWMH* init(Server* srv);

    xcb_atom_t      floating_win_atom;
    xcb_atom_t      tiling_win_atom;
    xcb_atom_t      wm_window_atom;
    xcb_timestamp_t timestamp;
    xcb_window_t    wm_window;

    bool acquire_timestamp();
    bool acquire_selection_screen(bool replace_wm);
};
