#include "atoms.h"
#include "config.h"
#include "error.h"
#include "logger.h"
#include "server.h"
#include <thread>

extern "C" {
#define explicit _explicit
#include <getopt.h>
#include <unistd.h>
#include <xcb/shape.h>
#include <xcb/xkb.h>
#include <xcb/randr.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#undef explicit
}

#define WM_SN_CLASS "cubewm-WM_Sn\0cubewm-WM_Sn"
#define WM_SN_NAME  "cubewm selection window"
static xcb_window_t main_window;

#define xmacro(name) xcb_atom_t name = 0;
ATOMS_XMACRO
xmacro(WM_SN_ATOM)
#undef xmacro

static void setup_atoms(const Server& srv)
{
#define xmacro(name) { name = srv.atom(#name); }
    ATOMS_XMACRO
#undef xmacro
}

static void setup_hints(const Server& srv)
{
    xcb_atom_t supported_atoms[] = {
#define xmacro(atom) atom,
        SUPPORTED_ATOMS_XMACRO
#undef xmacro
    };

    srv.change_atom_property(srv.root_window(), _NET_SUPPORTED, supported_atoms);

    static const char* name = "cube";

    srv.change_window_property(srv.root_window(), _NET_SUPPORTING_WM_CHECK, {&main_window, 1});
    srv.change_window_property(main_window, _NET_SUPPORTING_WM_CHECK, {&main_window, 1});
    srv.change_string_property(main_window, _NET_WM_NAME, {&name, 1});

    xcb_map_window(srv(), main_window);
}

static void setup_main_window(const Server& srv)
{
    main_window = xcb_generate_id(srv());

    int temp[] = {1};
    xcb_create_window(srv(), XCB_COPY_FROM_PARENT, main_window, srv.root_window(),
                      // Just in case i forgor
                      -1, -1, 1, 1, // dim (x, y, w, h)
                      0,            // border
                      XCB_WINDOW_CLASS_INPUT_ONLY, XCB_COPY_FROM_PARENT, XCB_CW_OVERRIDE_REDIRECT, temp);
    xcb_icccm_set_wm_class(srv(), main_window, sizeof(WM_SN_CLASS), WM_SN_CLASS);
    xcb_icccm_set_wm_name(srv(), main_window, XCB_ATOM_STRING, 8, sizeof(WM_SN_NAME) - 1, WM_SN_NAME);
}

static void acquire_selection_owner(const Server& srv, bool replace_wm)
{
    if (WM_SN_ATOM == XCB_NONE) {
        char* atom_name = xcb_atom_name_by_screen("WM", srv.default_screen);
        WM_SN_ATOM      = srv.atom(atom_name);
        free(atom_name);
    }

    auto* reply = xcb_get_selection_owner_reply(srv(), xcb_get_selection_owner(srv(), WM_SN_ATOM), 0);
    if (reply && reply->owner != XCB_NONE && !replace_wm) {
        free(reply);
        assert_runtime(false, "Another WM is running (Selection Owner)");
    }

    // Create the main window to use for selection owner
    setup_main_window(srv);
    
    // This will trigger selection clear event on another wm
    xcb_set_selection_owner(srv(), main_window, WM_SN_ATOM, srv.timestamp);

    // Wait for another wm to exit
    if (reply->owner != XCB_NONE) {
        int check_times = 10;
        while(true) {
            Log::debug("Waiting for another WM to exit: {}", check_times);

            auto* greply = xcb_get_geometry_reply(srv(), xcb_get_geometry(srv(), reply->owner), 0);
            if (greply) {
                free(greply);
            } else break;

            assert_runtime(check_times-- != 0, "The old window manager is not existing");

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s); // It's clear, sleep for 10 seconds
        }
    }

    free(reply);

    // Announce that we're the new selection owner
    char                        buf[32] = {0};
    xcb_client_message_event_t* event   = (xcb_client_message_event_t*)buf;

    event->response_type  = XCB_CLIENT_MESSAGE;
    event->window         = srv.root_window();
    event->format         = 32;
    event->type           = MANAGER;
    event->data.data32[0] = srv.timestamp;
    event->data.data32[1] = WM_SN_ATOM;
    event->data.data32[2] = main_window;

    xcb_send_event(srv(), 0, srv.root_window(), XCB_EVENT_MASK_STRUCTURE_NOTIFY, (char*)event);
}

static void ensure_no_other_wm(const Server& srv)
{
    auto* reply = xcb_request_check(srv(), xcb_change_window_attributes_checked(srv(), srv.root_window(), XCB_CW_EVENT_MASK, &Config::ROOT_EVENT_MASK));
    if (reply) {
        free(reply);
        assert_runtime(false, "Another WM is running (X server)");
    }
}

static void load_xkb(const Server& srv)
{
    const auto* reply = xcb_get_extension_data(srv(), &xcb_xkb_id);

    if (!reply->present) {
        Log::error("xcb is not present on this server");
        return;
    }

    // Meh, can we simplify this?
    xcb_xkb_use_extension(srv(), XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
    xcb_xkb_select_events(srv(), XCB_XKB_ID_USE_CORE_KBD,
                          XCB_XKB_EVENT_TYPE_STATE_NOTIFY | XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
                              XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY,
                          0,
                          XCB_XKB_EVENT_TYPE_STATE_NOTIFY | XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
                              XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY,
                          0xff, 0xff, NULL);

    const uint32_t flags = XCB_XKB_PER_CLIENT_FLAG_GRABS_USE_XKB_STATE |
                           XCB_XKB_PER_CLIENT_FLAG_LOOKUP_STATE_WHEN_GRABBED |
                           XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT;
    auto* client_flags = xcb_xkb_per_client_flags_reply(
        srv(), xcb_xkb_per_client_flags(srv(), XCB_XKB_ID_USE_CORE_KBD, flags, flags, 0, 0, 0), NULL);

    if (!client_flags || !(client_flags->value & flags)) Log::error("Could not get xkb client flags");

    free(client_flags);

    Config::xkb_support = reply->present;
    Config::xkb_base    = reply->first_event;
}

static void load_shape(const Server& srv)
{
    const auto* reply = xcb_get_extension_data(srv(), &xcb_shape_id);

    if (!reply->present) {
        Log::error("shape is not present on this server");
        return;
    }

    auto* version = xcb_shape_query_version_reply(srv(), xcb_shape_query_version(srv()), NULL);

    Config::shape_support = version && version->minor_version >= 1;
    Config::shape_base    = reply->first_event;

    free(version);
}

static void load_randr(const Server& srv)
{
    xcb_generic_error_t* err;

    const auto* reply = xcb_get_extension_data(srv(), &xcb_randr_id);
    if (!reply->present) {
        Log::error("RandR is not present on this server");
        return;
    }

    auto* version = xcb_randr_query_version_reply(
        srv(), xcb_randr_query_version(srv(), XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION), &err);
    if (err) {
        Log::error("Could not query RandR version: X11 err code {}", err->error_code);
        free(err);
        return;
    }

    if (version->major_version < 1 || version->minor_version < 5) {
        Log::error("Must have RandR version 1.5+");
        free(version);
        return;
    }

    free(version);
    
    Config::randr_support = true; 
    Config::randr_base    = reply->first_event;
}

static void setup_extensions(const Server& srv)
{
    xcb_prefetch_extension_data(srv(), &xcb_xkb_id);
    xcb_prefetch_extension_data(srv(), &xcb_shape_id);
    xcb_prefetch_extension_data(srv(), &xcb_randr_id);
    
    load_xkb(srv);
    load_shape(srv);
    load_randr(srv);
}

int main(int argc, char* const argv[])
{
    static struct option options[] = {
        {"help",    no_argument, 0, 'h'},
        {"replace", no_argument, 0, 'r'}
    };
    int option_index, opt = 0;

    while ((opt = getopt_long(argc, argv, "hr", options, &option_index)) != -1) {
        switch (opt) {
        case 'h':
            // Print help
            Log::info("Still WIP");
            return 0;
        case 'r':
            Config::replace_wm = true;
            break;
        }
    }

    Log::info("Starting cubewm...");

    try {
        Server& srv = Server::instance();
       
        Log::debug("Setting up atoms");
        setup_atoms(srv);

        Log::debug("Acquiring selection owner");
        acquire_selection_owner(srv, Config::replace_wm);

        Log::debug("Checking for another wm");
        ensure_no_other_wm(srv);

        Log::debug("Setting up hints");
        setup_hints(srv);

        Log::debug("Looking up for extensions");
        setup_extensions(srv);

        srv.run();
    } catch (const std::exception& e) {
        Log::error(e.what());
    }

    return 0;
}
