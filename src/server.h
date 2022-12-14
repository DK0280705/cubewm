#pragma once
#include <unordered_map>
extern "C" {
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xproto.h>
}

class Con;
class Win;

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
    Server(xcb_connection_t* conn, int screen_id);

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

    // extensions
    uint8_t xkb_support;
    uint8_t xkb_base;

    uint8_t shape_support;
    uint8_t shape_base;

    uint8_t randr_support;
    uint8_t randr_base;

    xcb_connection_t* conn;

    int           screen_id;
    xcb_screen_t* screen;

    class Event_handler* eh;
    class EWMH*          ewmh;
    class Root*          root;

    void run();

    xcb_atom_t atom(const char* atom_name);

    void manage_window(xcb_window_t id);

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
