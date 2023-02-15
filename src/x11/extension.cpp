#include "extension.h"
#include "../connection.h"
#include "../logger.h"
#include "x11.h"
#include <xcb/randr.h>
#include <xcb/shape.h>
// C++ unfriendly
#define explicit _explicit
#include <xcb/xkb.h>
#undef explicit


namespace X11 {
namespace extension {

Extension xkb(0, false);
Extension xrandr(0, false);
Extension xshape(0, false);

static Extension init_xkb(const Connection& conn)
{
    const auto* reply = xcb_get_extension_data(conn, &xcb_xkb_id);

    if (!reply->present) {
        logger::error("xkb is not present on this server");
        return Extension(0, false);
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
        logger::error("Could not get xkb client flags");

    if (client_flags) free(client_flags);

    return Extension(reply->first_event, reply->present);
}

static Extension init_xrandr(const Connection& conn)
{
    const auto* reply = xcb_get_extension_data(conn, &xcb_randr_id);
    if (!reply->present) {
        logger::error("xrandr is not present on this server");
        return Extension(0, false);
    }

    xcb_generic_error_t* err = nullptr;
    auto version = memory::c_own(xcb_randr_query_version_reply(
        conn,
        xcb_randr_query_version(conn,
                                XCB_RANDR_MAJOR_VERSION,
                                XCB_RANDR_MINOR_VERSION),
        &err));

    if (err) {
        logger::error("Could not query RandR version: err code {}", err->error_code);
        return Extension(0, false);
    }

    if (version->major_version < 1 || version->minor_version < 5) {
        logger::error("Must have RandR version 1.5+");
        return Extension(0, false);
    }

    return Extension(true, reply->first_event);
}

static Extension init_xshape(const Connection& conn)
{
    const auto* reply = xcb_get_extension_data(conn, &xcb_shape_id);

    if (!reply->present) {
        logger::error("xshape is not present on this server");
        return Extension(0, false);
    }

    auto version = memory::c_own(xcb_shape_query_version_reply(
        conn, xcb_shape_query_version(conn), NULL));

    return Extension(reply->first_event, version && version->minor_version >= 1);
}

void init()
{
    const Connection& conn = X11::_conn();

    xkb = init_xkb(conn);
    xrandr = init_xrandr(conn);
    xshape = init_xshape(conn);
}

}
}
