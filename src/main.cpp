#include "atoms.h"
#include "config.h"
#include "connection.h"
#include "error.h"
#include "logger.h"
#include "server.h"
#include "xwrap.h"
#include <csignal>
#include <thread>

extern "C" {
#include <getopt.h>
#include <unistd.h>
#include <xcb/xcb_atom.h>
#include <xcb/xproto.h>
}

static void setup_hints(const Connection& conn, const xcb_window_t main_window)
{
    xcb_atom_t supported_atoms[] = {
#define xmacro(atom) Atom::atom,
        SUPPORTED_ATOMS_XMACRO
#undef xmacro
    };

    XWrap::change_atom_property(conn.screen()->root,
                                Atom::_NET_SUPPORTED,
                                supported_atoms);

    // Setup main window property
    static const char* name = "cube";

    XWrap::change_window_property(conn.screen()->root,
                                  Atom::_NET_SUPPORTING_WM_CHECK,
                                  {&main_window, 1});
    XWrap::change_window_property(main_window,
                                  Atom::_NET_SUPPORTING_WM_CHECK,
                                  {&main_window, 1});
    XWrap::change_utf8string_property(main_window,
                                      Atom::_NET_WM_NAME,
                                      {&name, 1});

    xcb_map_window(conn, main_window);
}

static xcb_window_t setup_main_window(const Connection& conn)
{
    static const xcb_window_t main_window = xcb_generate_id(conn);

    const int temp[] = {1};
    xcb_create_window(conn,
                      XCB_COPY_FROM_PARENT,
                      main_window,
                      conn.screen()->root,
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

    XWrap::change_string_property(main_window, XCB_ATOM_WM_CLASS, WM_SN_CLASS);
    XWrap::change_string_property(main_window, XCB_ATOM_WM_NAME, WM_SN_NAME);

    return main_window;
}

static void acquire_first_timestamp(Connection& conn)
{
    // Initiate requests
    xcb_grab_server(conn);
    XWrap::change_window_attributes(conn.screen()->root,
                                    XCB_CW_EVENT_MASK,
                                    {{XCB_EVENT_MASK_PROPERTY_CHANGE}});
    XWrap::change_cardinal_property(conn.screen()->root,
                                    XCB_ATOM_SUPERSCRIPT_X,
                                    {},
                                    XWrap::CP::Append);
    xcb_ungrab_server(conn);

    xcb_flush(conn);

    xcb_generic_event_t* event = nullptr;
    while ((event = xcb_wait_for_event(conn)))
        if ((event->response_type & 0x7F) == XCB_PROPERTY_NOTIFY) {
            conn.timestamp.update(&((xcb_property_notify_event_t*)event)->time);
            free(event);
            return;
        } else free(event);
}

static void acquire_selection_owner(const Connection&  conn,
                                    const xcb_window_t main_window,
                                    const bool         replace_wm)
{
    std::unique_ptr<xcb_get_selection_owner_reply_t, void(*)(void*)> reply(
        xcb_get_selection_owner_reply(
            conn, xcb_get_selection_owner(conn, Atom::WM_SN), 0),
        free);
    assert_runtime(!(reply && reply->owner != XCB_NONE && !replace_wm), "Another WM is running (Selection Owner)");

    // This will trigger selection clear event on another wm
    xcb_set_selection_owner(conn, main_window, Atom::WM_SN, conn.timestamp);

    // Wait for another wm to exit
    if (reply->owner != XCB_NONE) {
        unsigned int check_times = 10;
        while (true) {
            Log::debug("Waiting for another WM to exit: {}", check_times);

            auto* greply = xcb_get_geometry_reply(
                conn, xcb_get_geometry(conn, reply->owner), 0);
            if (greply) free(greply);
            else break;

            assert_runtime(check_times-- != 0,
                           "The old window manager is not existing");

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s); // It's clear, sleep for 10 seconds
        }
    }

    // Announce that we're the new selection owner
    const xcb_client_message_event_t event = {
        .response_type = XCB_CLIENT_MESSAGE,
        .format        = 32,
        .window        = conn.screen()->root,
        .type          = Atom::MANAGER,
        .data = {.data32 = {conn.timestamp, Atom::WM_SN, main_window}}};

    XWrap::send_event(conn.screen()->root,
                      XCB_EVENT_MASK_STRUCTURE_NOTIFY,
                      (const char*)(&event));
}

static void ensure_no_other_wm(const Connection& conn)
{
    assert_runtime(XWrap::check_error(XWrap::change_window_attributes(
                       conn.screen()->root,
                       XCB_CW_EVENT_MASK,
                       {{Config::ROOT_EVENT_MASK}})),
                   "Another WM is running (X server)");
}

static Server* _srv_p = nullptr; // NOLINT

int main(int argc, char* const argv[])
{
    static const std::array<struct option, 3> options {
        {
         {"help", no_argument, 0, 'h'},
         {"replace", no_argument, 0, 'r'},
         {"use-xinerama", no_argument, 0, 'x'},
         }
    };
    int option_index = 0, opt = 0;

    while (
        (opt = getopt_long(argc, argv, "hrx", options.data(), &option_index)) !=
        -1) {
        switch (opt) {
        case 'h':
            // Print help
            Log::info("Still WIP");
            return 0;
        case 'r':
            Config::replace_wm = true;
            break;
        case 'x':
            Config::xinerama_enabled = true;
            break;
        default:
            Log::error("Unrecognized options");
            return 1;
        }
    }

    Log::info("Starting cubewm...");

    try {
        Connection& conn = Connection::init();

        // Init first, or funny things happen
        Config::init(conn);
        Atom::init(conn);
        XWrap::init(conn);

        // We have to cope with the ICCCM
        acquire_first_timestamp(conn);
        Log::debug("Last timestamp: {}", conn.timestamp);

        const xcb_window_t main_window = setup_main_window(conn);
        acquire_selection_owner(conn, main_window, Config::replace_wm);
        Log::debug("Selection owner acquired");

        ensure_no_other_wm(conn);
        Log::debug("No other wm ensured. Save to run.");

        Config::load_config();
        Config::load_extensions();

        Log::debug("Setting up hints");
        setup_hints(conn, main_window);

        Server& srv = Server::init(conn);

        // When you have to deal with C
        _srv_p        = &srv;
        auto shandler = [](int) { _srv_p->stop(); };
        std::signal(SIGINT, shandler);
        std::signal(SIGQUIT, shandler);
        std::signal(SIGTERM, shandler);
        std::signal(SIGCHLD, [](int) {});

        Rectangle rect = srv.default_rect();
        Log::debug("Root rect -> x: {}, y: {}, width: {}, height: {}",
                   rect.x,
                   rect.y,
                   rect.width,
                   rect.height);

        srv.start();
    } catch (const std::exception& e) {
        Log::error(e.what());
        return 1;
    }

    return 0;
}
