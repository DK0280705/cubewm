#include "config.h"
#include "connection.h"
#include "error.h"
#include "logger.h"

extern "C" {
#define explicit _explicit
#include <xcb/randr.h>
#include <xcb/shape.h>
#include <xcb/xkb.h>
#undef explicit
}

namespace Config {
bool replace_wm  = false;

bool xkb_support   = false;
bool shape_support = false;
bool randr_support = false;

uint8_t xkb_base   = 0;
uint8_t shape_base = 0;
uint8_t randr_base = 0;

bool xinerama_enabled = false;
bool randr_enabled = true;

static const Connection* conn = nullptr;
void init(const Connection& conn)
{
    if (Config::conn) throw Init_error("Config");
    Config::conn = &conn;

    xcb_prefetch_extension_data(conn, &xcb_xkb_id);
    xcb_prefetch_extension_data(conn, &xcb_shape_id);
    xcb_prefetch_extension_data(conn, &xcb_randr_id);
}

void load_config()
{
}

static void load_xkb(const Connection& conn)
{
    const auto* reply = xcb_get_extension_data(conn, &xcb_xkb_id);

    if (!reply->present) {
        Log::error("xkb is not present on this server");
        return;
    }

    // Meh, can we simplify this?
    xcb_xkb_use_extension(conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
    xcb_xkb_select_events(conn,
                          XCB_XKB_ID_USE_CORE_KBD,
                          XCB_XKB_EVENT_TYPE_STATE_NOTIFY |
                              XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
                              XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY,
                          0,
                          XCB_XKB_EVENT_TYPE_STATE_NOTIFY |
                              XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
                              XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY,
                          0xff,
                          0xff,
                          NULL);

    const uint32_t flags = XCB_XKB_PER_CLIENT_FLAG_GRABS_USE_XKB_STATE |
                           XCB_XKB_PER_CLIENT_FLAG_LOOKUP_STATE_WHEN_GRABBED |
                           XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT;
    auto* client_flags = xcb_xkb_per_client_flags_reply(
        conn,
        xcb_xkb_per_client_flags(
            conn, XCB_XKB_ID_USE_CORE_KBD, flags, flags, 0, 0, 0),
        NULL);

    if (!client_flags || !(client_flags->value & flags))
        Log::error("Could not get xkb client flags");

    free(client_flags);

    Config::xkb_support = reply->present;
    Config::xkb_base    = reply->first_event;
}

static void load_shape(const Connection& conn)
{
    const auto* reply = xcb_get_extension_data(conn, &xcb_shape_id);

    if (!reply->present) {
        Log::error("shape is not present on this server");
        return;
    }

    auto* version = xcb_shape_query_version_reply(conn,
                                                  xcb_shape_query_version(conn),
                                                  NULL);

    Config::shape_support = version && version->minor_version >= 1;
    Config::shape_base    = reply->first_event;

    free(version);
}

static void load_randr(const Connection& conn)
{
    xcb_generic_error_t* err = nullptr;

    const auto* reply = xcb_get_extension_data(conn, &xcb_randr_id);
    if (!reply->present) {
        Log::error("RandR is not present on this server");
        return;
    }

    auto* version = xcb_randr_query_version_reply(
        conn,
        xcb_randr_query_version(conn,
                                XCB_RANDR_MAJOR_VERSION,
                                XCB_RANDR_MINOR_VERSION),
        &err);
    if (err) {
        Log::error("Could not query RandR version: X11 err code {}",
                   err->error_code);
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

void load_extensions()
{    
    load_xkb(*conn);
    load_shape(*conn);
    load_randr(*conn);
}

}
