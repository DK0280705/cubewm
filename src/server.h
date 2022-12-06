#pragma once
#include <xcb/xcb_cursor.h>
extern "C" {
#include <xcb/xcb.h>
#include <xcb/xproto.h>
}

enum XCursor : unsigned int
{
    XCURSOR_POINTER = 0,
    XCURSOR_RESIZE_HORIZONTAL,
    XCURSOR_RESIZE_VERTICAL,
    XCURSOR_TOP_LEFT_CORNER,
    XCURSOR_TOP_RIGHT_CORNER,
    XCURSOR_BOTTOM_LEFT_CORNER,
    XCURSOR_BOTTOM_RIGHT_CORNER,
    XCURSOR_WATCH,
    XCURSOR_MOVE,
    XCURSOR_MAX
};

class Server
{
    void _check_another_wm();
    void _acquire_atoms();
    void _acquire_timestamp();
    void _acquire_wm_sn();

    void _load_cursors();

    Server();

public:
    Server(const Server& s)  = delete;
    Server(const Server&& s) = delete;

    ~Server();

    static Server* init();

    int screen_id;

    xcb_connection_t* conn;
    xcb_timestamp_t   timestamp;
    xcb_atom_t        wm_sn;
    xcb_window_t      wm_sn_owner;

    xcb_screen_t* screen;

    void    run();

    xcb_window_t create_window();

    struct Cursor
    {
        Cursor(const Server& srv) noexcept;
        void          operator=(enum XCursor c);
        xcb_cursor_t& operator[](enum XCursor c);

    private:
        const Server&         _srv;
        xcb_cursor_context_t* _ctx = nullptr;
        xcb_cursor_t          _cursors[XCURSOR_MAX];
        friend class Server;
    } cursor;
};
