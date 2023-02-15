#include "x11.h"
#include "atom.h"
#include "constant.h"
#include "extension.h"
#include "window.h"
#include "../config.h"
#include "../connection.h"
#include "../logger.h"
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <chrono>
#include <thread>

namespace X11 {
static Connection* _pconn = nullptr;

static void acquire_first_timestamp(Connection& conn)
{
    // Initiate requests
    xcb_grab_server(conn);
    window::change_attributes(conn.xscreen()->root,
                              XCB_CW_EVENT_MASK,
                              {{XCB_EVENT_MASK_PROPERTY_CHANGE}});
    window::change_property(window::prop::append,
                            conn.xscreen()->root,
                            XCB_ATOM_SUPERSCRIPT_X,
                            XCB_ATOM_CARDINAL,
                            std::span<const uint32_t, 0>{});
    xcb_ungrab_server(conn);

    xcb_flush(conn);

    xcb_generic_event_t* event = nullptr;
    while ((event = xcb_wait_for_event(conn)))
        if ((event->response_type & 0x7F) == XCB_PROPERTY_NOTIFY) {
            conn.timestamp(((xcb_property_notify_event_t*)event)->time);
            free(event);
            return;
        } else free(event);
}

static void acquire_selection_owner(const Connection&  conn,
                                    const xcb_window_t main_window,
                                    const bool         replace_wm)
{
    auto reply = memory::c_own<xcb_get_selection_owner_reply_t>(
        xcb_get_selection_owner_reply(
            conn, xcb_get_selection_owner(conn, atom::WM_SN), 0));
    assert_runtime(!(reply && reply->owner != XCB_NONE && !replace_wm), "Another WM is running (Selection Owner)");

    // This will notify selection clear event on another wm
    xcb_set_selection_owner(conn, main_window, atom::WM_SN, conn.timestamp());

    // Wait for another wm to exit
    if (reply->owner != XCB_NONE) {
        unsigned int check_times = 10;
        while (true) {
            logger::info("Waiting for another WM to exit: {}", check_times);

            auto* greply = xcb_get_geometry_reply(
                conn, xcb_get_geometry(conn, reply->owner), 0);
            if (greply) free(greply);
            else break;

            assert_runtime(check_times-- != 0, "Timeout reached");

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s); // It's clear, sleep for 10 seconds
        }
    }

    // Announce that we're the new selection owner
    const xcb_client_message_event_t event = {
        .response_type = XCB_CLIENT_MESSAGE,
        .format        = 32,
        .window        = conn.xscreen()->root,
        .type          = atom::MANAGER,
        .data = {.data32 = {conn.timestamp(), atom::WM_SN, main_window}}};

    xcb_send_event(conn, 0, conn.xscreen()->root, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char*)&event);
}

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

    acquire_first_timestamp(conn);
    logger::debug("First timestamp: {}", conn.timestamp());

    const xcb_window_t main_window = setup_main_window(conn);
    acquire_selection_owner(conn, main_window, config::replace_wm);
    logger::debug("Selection owner acquired");

    try {
        window::change_attributes_c(conn.xscreen()->root,
                                    XCB_CW_EVENT_MASK,
                                    {{constant::ROOT_EVENT_MASK}});
    } catch(const std::runtime_error& err) {
        // Rethrow with different message
        throw std::runtime_error("Another WM is running (X Server)");
    }

    extension::init();
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
