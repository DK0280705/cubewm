#pragma once
extern "C" {
#define SN_API_NOT_YET_FROZEN
#include <libsn/sn.h>

#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_keysyms.h>
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

class Server final // kinda useless, but just for fancy.
{
    Server();

    class Event_handler* _eh;

    void _acquire_atoms();
    void _acquire_timestamp();
    void _acquire_wm_sn();

    void _check_another_wm() const;

    void _load_sn();
    void _load_cursors();
    void _load_xkb();
    void _load_shape();
    void _load_randr();

public:
    Server(const Server& s)  = delete;
    Server(const Server&& s) = delete;

    ~Server();

    static Server* init();

    int screen_id;

    // extensions
    uint8_t xkb_support;
    uint8_t xkb_base;

    uint8_t shape_support;
    uint8_t shape_base;

    uint8_t randr_support;
    uint8_t randr_base;

    xcb_key_symbols_t* keysyms;

    SnDisplay* sndisplay;

    xcb_connection_t* conn;
    xcb_timestamp_t   timestamp;
    xcb_atom_t        wm_sn;
    xcb_window_t      wm_sn_owner;

    xcb_screen_t* screen;

    void run();

    xcb_window_t create_window();

    struct Cursor
    {
        Cursor(const Server& srv) noexcept;
        // These are not actual setter and getter
        // This is weird ones
        void          operator=(enum XCursor c) const;
        xcb_cursor_t& operator[](enum XCursor c);

    private:
        const Server&         _srv;
        xcb_cursor_context_t* _ctx = nullptr;
        xcb_cursor_t          _cursors[XCURSOR_MAX];
        friend class Server;
    } cursor;
};
