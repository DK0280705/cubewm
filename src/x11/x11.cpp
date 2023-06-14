#include "x11.h"
#include "atom.h"
#include "event.h"
#include "extension.h"
#include "ewmh.h"
#include "window.h"
#include "../config.h"
#include "../logger.h"
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <chrono>
#include <thread>

namespace X11 {

[[nodiscard]]
static auto _get_timestamp(const Connection& conn) noexcept -> xcb_timestamp_t
{
    // Initiate requests
    xcb_grab_server(conn);
    const uint32_t mask[] = { XCB_EVENT_MASK_PROPERTY_CHANGE };
    window::change_attributes(root_window_id(conn),
                              XCB_CW_EVENT_MASK,
                              std::span{mask});
    window::change_property(root_window_id(conn),
                            window::prop::append,
                            XCB_ATOM_SUPERSCRIPT_X,
                            XCB_ATOM_CARDINAL,
                            std::span<const uint32_t, 0>{});
    xcb_ungrab_server(conn);

    conn.flush();

    xcb_generic_event_t* event = nullptr;
    auto _ = memory::finally([&]() { if (event) free(event); });

    while ((event = xcb_wait_for_event(conn)))
        if ((event->response_type & 0x7F) == XCB_PROPERTY_NOTIFY) {
            return ((xcb_property_notify_event_t*)event)->time;
        } else free(event);
    std::unreachable();
}

[[nodiscard]]
static auto _get_selection_owner(const Connection& conn) -> xcb_window_t
{
    auto reply = memory::c_own(xcb_get_selection_owner_reply(
            conn,
            xcb_get_selection_owner(conn, atom::WM_SN),
            nullptr));
    return (reply) ? reply->owner : XCB_NONE;
}

[[nodiscard]]
static auto _create_main_window(const Connection& conn) -> xcb_window_t
{
    xcb_window_t main_window = xcb_generate_id(conn);

    const int ov_rdr[] = {1};
    xcb_create_window(conn, XCB_COPY_FROM_PARENT, main_window, root_window_id(conn),
                      -1, -1, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_ONLY, XCB_COPY_FROM_PARENT,
                      XCB_CW_OVERRIDE_REDIRECT, static_cast<const void*>(ov_rdr));

    window::change_property(main_window,
                            window::prop::replace,
                            XCB_ATOM_WM_CLASS,
                            XCB_ATOM_STRING,
                            std::span{config::WM_SN_CLASS});
    window::change_property(main_window,
                            window::prop::replace,
                            XCB_ATOM_WM_NAME,
                            XCB_ATOM_STRING,
                            std::span{config::WM_NAME});

    return main_window;
}

static void _acquire_selection_owner(const Connection&  conn,
                                     const xcb_window_t main_window,
                                     const xcb_window_t previous_owner)
{
    // This will notify selection clear event on another wm
    xcb_set_selection_owner(conn, main_window, atom::WM_SN, Timestamp::get());

    // Wait for another wm to exit if previous owner exists
    if (previous_owner != XCB_NONE) {
        unsigned int check_times = 10;
        while (true) {
            logger::info("Waiting for another WM to exit: {}", check_times);

            auto* greply = xcb_get_geometry_reply(
                conn, xcb_get_geometry(conn, previous_owner), 0);
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
        .sequence      = 0,
        .window        = root_window_id(conn),
        .type          = atom::MANAGER,
        .data = {.data32 = {Timestamp::get(), atom::WM_SN, main_window}}};

    xcb_send_event(conn, 0, root_window_id(conn), XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char*)&event);
}

static const X11::Connection* _pconnection = nullptr;
static xcb_window_t           _main_window = 0;

void init(const X11::Connection& conn)
{
    _pconnection = &conn;

    // Initialize atoms, get all runtime defined atoms.
    atom::init(conn);

    // Get timestamp
    xcb_timestamp_t timestamp = _get_timestamp(conn);
    Timestamp::update(timestamp);
    logger::debug("First timestamp: {}", Timestamp::get());

    // Get current selection owner
    xcb_window_t prev_owner = _get_selection_owner(conn);
    assert_runtime<Display_error>(prev_owner == XCB_NONE || config::replace_wm,
                                  "Another WM is running (Selection Owner)");
    logger::debug("Current selection owner: {:#x}", prev_owner);

    // Set _NET_SUPPORTED hints
    xcb_atom_t supported_atoms[] = {
#define xmacro(a) atom::a,
   SUPPORTED_ATOMS_XMACRO
#undef xmacro
    };
    ewmh::update_net_supported(supported_atoms);

    // Create main window.
    _main_window = _create_main_window(conn);

    // Try to acquire selection owner and replace current window manager if it's exist.
    _acquire_selection_owner(conn, _main_window, prev_owner);
    logger::debug("Selection owner acquired, main window: {:#x}", _main_window);
    xcb_map_window(conn, _main_window);

    // Set _NET_SUPPORTING_WM_CHECK hints
    ewmh::update_net_supporting_wm_check(_main_window);

    // Initialize events for root window.
    event::init(conn);

    // Initialize extensions: XRandR, XShape, Xinerama, XKB
    extension::init(conn);
}

namespace detail {

auto conn() noexcept -> const Connection&
{
    assert_debug(_pconnection, "X11 is not initialized yet");
    // Nahh if statement is stupid.
    // This is already stupid, let the application blow up
    // if someone put X11::_conn() before X11::init(conn)
    return *_pconnection;
}

auto root_window_id() noexcept -> xcb_window_t
{
    assert_debug(_pconnection, "X11 is not initialized yet");
    return X11::root_window_id(*_pconnection);
}

auto main_window_id() noexcept -> xcb_window_t
{
    assert_debug(_main_window != 0, "Window manager is not initialized yet");
    return _main_window;
}

void check_error(const xcb_void_cookie_t& cookie)
{
    auto reply = memory::c_own(xcb_request_check(X11::detail::conn(), cookie));
    assert_runtime(!reply, "Change property failed");
}

}

}
