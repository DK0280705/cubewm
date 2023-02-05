#include "x11.h"
#include "atom.h"
#include "window.h"
#include "../connection.h"
#include <xcb/xcb.h>
#include <xcb/xproto.h>

namespace X11 {
static Connection* _pconn = nullptr;

static void setup_hints(const Connection& conn, const xcb_window_t main_window)
{
    xcb_atom_t supported_atoms[] = {
#define xmacro(a) atom::a,
        SUPPORTED_ATOMS_XMACRO
#undef xmacro
    };

    window::change_property(window::prop::replace,
                            conn.xscreen()->root,
                            atom::_NET_SUPPORTED,
                            XCB_ATOM_ATOM,
                            std::span{supported_atoms});

    // Setup main window property
    static const char* name = "cube";

    window::change_property(window::prop::replace,
                            conn.xscreen()->root,
                            atom::_NET_SUPPORTING_WM_CHECK,
                            XCB_ATOM_WINDOW,
                            std::span{&main_window, 1});
    window::change_property(window::prop::replace,
                            main_window,
                            atom::_NET_SUPPORTING_WM_CHECK,
                            XCB_ATOM_WINDOW,
                            std::span{&main_window, 1});
    window::change_property(window::prop::replace,
                            main_window,
                            atom::_NET_WM_NAME,
                            atom::UTF8_STRING,
                            std::span{&name, 1});

    xcb_map_window(conn, main_window);
}

static xcb_window_t setup_main_window(const Connection& conn)
{
    xcb_window_t main_window = xcb_generate_id(conn);

    const int temp[] = {1};
    xcb_create_window(conn,
                      XCB_COPY_FROM_PARENT,
                      main_window,
                      conn.xscreen()->root,
                      // Just in case i forgor
                      -1,
                      -1,
                      1,
                      1, // dim (x, y, w, h)
                      0, // border
                      XCB_WINDOW_CLASS_INPUT_ONLY,
                      XCB_COPY_FROM_PARENT,
                      XCB_CW_OVERRIDE_REDIRECT,
                      static_cast<const void*>(temp));

    static constexpr const char WM_SN_CLASS[] = "cubewm-WM_Sn\0cubewm-WM_Sn";
    static constexpr const char WM_SN_NAME[]  = "cubewm selection window";

    window::change_property(window::prop::replace,
                            main_window,
                            XCB_ATOM_WM_CLASS,
                            XCB_ATOM_STRING,
                            std::span{WM_SN_CLASS});
    window::change_property(window::prop::replace,
                            main_window,
                            XCB_ATOM_WM_NAME,
                            XCB_ATOM_STRING,
                            std::span{WM_SN_NAME});

    return main_window;
}

void init(Connection& conn)
{
    _pconn = &conn;
    
    atom::init();
    const xcb_window_t main_window = setup_main_window(conn);
    setup_hints(conn, main_window);
}

Connection& _conn()
{
    // Nahh if statement is stupid.
    // This is already stupid, let the application blow up
    // if someone put X11::_conn() before X11::init(conn)
    return *_pconn;
}
}
