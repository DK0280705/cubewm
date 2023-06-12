#include "extension.h"
#include "connection.h"

#include "../logger.h"
#include "../helper/memory.h"

#include <xcb/randr.h>
#include <xcb/shape.h>
// C++ unfriendly
#define explicit _explicit
#include <xcb/xkb.h>
#undef explicit


namespace X11::extension {

static XKB_extension    _xkb;
static Xrandr_extension _xrandr;
static Xshape_extension _xshape;

auto xkb() noexcept -> const XKB_extension&
{
    return _xkb;
}
auto xrandr() noexcept -> const Xrandr_extension&
{
    return _xrandr;
}
auto xshape() noexcept -> const Xshape_extension&
{
    return _xshape;
}

static auto _init_xkb(const Connection& conn) -> XKB_extension
{
    const auto* reply = xcb_get_extension_data(conn, &xcb_xkb_id);

    if (!reply->present) {
        logger::error("xkb is not present on this server");
        return {};
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
                          nullptr);

    const uint32_t flags = XCB_XKB_PER_CLIENT_FLAG_GRABS_USE_XKB_STATE |
                           XCB_XKB_PER_CLIENT_FLAG_LOOKUP_STATE_WHEN_GRABBED |
                           XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT;
    auto client_flags = memory::c_own(xcb_xkb_per_client_flags_reply(
        conn,
        xcb_xkb_per_client_flags(
            conn, XCB_XKB_ID_USE_CORE_KBD, flags, flags, 0, 0, 0),
        nullptr));

    if (!client_flags || !(client_flags->value & flags))
        logger::error("Could not get xkb client flags");

    return {reply->first_event, (bool)reply->present};
}

static auto _init_xrandr(const Connection& conn) -> Xrandr_extension
{
    const auto* reply = xcb_get_extension_data(conn, &xcb_randr_id);
    if (!reply->present) {
        logger::error("xrandr is not present on this server");
        return {};
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
        return {};
    }

    if (version->major_version < 1 || version->minor_version < 2) {
        logger::error("Must have RandR version 1.2+");
        return {};
    }

    bool have_randr_13 = version->minor_version >= 3;
    bool have_randr_15 =
#if XCB_RANDR_MAJOR_VERSION > 1 || XCB_RANDR_MINOR_VERSION >= 5
        version->minor_version >= 5;
#else
        false;
#endif

    return {{reply->first_event, true}, have_randr_13, have_randr_15};
}

static auto _init_xshape(const Connection& conn) -> Xshape_extension
{
    const auto* reply = xcb_get_extension_data(conn, &xcb_shape_id);

    if (!reply->present) {
        logger::error("xshape is not present on this server");
        return {0, false};
    }

    auto version = memory::c_own(xcb_shape_query_version_reply(
        conn, xcb_shape_query_version(conn), nullptr));

    return {reply->first_event, version && version->minor_version >= 1};
}

void init(const Connection& conn)
{
    _xkb    = _init_xkb(conn);
    _xrandr = _init_xrandr(conn);
    _xshape = _init_xshape(conn);
}

} // namespace X11::extension
