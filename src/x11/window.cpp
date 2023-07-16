#include "window.h"
#include "atom.h"
#include "event.h"
#include "ewmh.h"
#include "extension.h"
#include "xkb.h"
#include "x11.h"

#include "../config.h"
#include "../logger.h"

#include <limits>
#include <xcb/shape.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xproto.h>

namespace X11 {

// Window information fetcher
namespace window::detail {

static void fetch_name(const xcb_window_t window_id, std::string& name)
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::detail::conn(),
            xcb_get_property(X11::detail::conn(), false,
                             window_id, X11::atom::_NET_WM_NAME,
                             XCB_GET_PROPERTY_TYPE_ANY, 0, 128),
            nullptr));
    if (!prop) return;

    name = std::string(
        reinterpret_cast<char*>(xcb_get_property_value(prop.get())),
        xcb_get_property_value_length(prop.get()));
}

static void fetch_type(const xcb_window_t window_id, xcb_atom_t& type)
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::detail::conn(),
            xcb_get_property(X11::detail::conn(), false, window_id,
                             X11::atom::_NET_WM_WINDOW_TYPE,
                             XCB_GET_PROPERTY_TYPE_ANY, 0,
                             std::numeric_limits<uint32_t>::max()),
            nullptr));
    if (!prop) return;

    // Return the very first supported atom
    const xcb_atom_t* atoms  = reinterpret_cast<xcb_atom_t*>(xcb_get_property_value(prop.get()));
    const std::size_t length = xcb_get_property_value_length(prop.get()) / sizeof(xcb_atom_t);
    for (const auto& atom : std::span{atoms, length})
        if (atom == X11::atom::_NET_WM_WINDOW_TYPE_DESKTOP ||
            atom == X11::atom::_NET_WM_WINDOW_TYPE_DOCK    ||
            atom == X11::atom::_NET_WM_WINDOW_TYPE_TOOLBAR ||
            atom == X11::atom::_NET_WM_WINDOW_TYPE_MENU    ||
            atom == X11::atom::_NET_WM_WINDOW_TYPE_DIALOG  ||
            atom == X11::atom::_NET_WM_WINDOW_TYPE_NORMAL  ||
            atom == X11::atom::_NET_WM_WINDOW_TYPE_TOOLTIP ||
            atom == X11::atom::_NET_WM_WINDOW_TYPE_SPLASH  ||
            atom == X11::atom::_NET_WM_WINDOW_TYPE_UTILITY ||
            atom == X11::atom::_NET_WM_WINDOW_TYPE_NOTIFICATION) {
            type = atom;
            return;
        }
}

static void fetch_role(const xcb_window_t window_id, std::string& role)
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::detail::conn(),
            xcb_get_property(X11::detail::conn(), false, window_id,
                             X11::atom::WM_WINDOW_ROLE,
                             XCB_GET_PROPERTY_TYPE_ANY, 0, 128),
            nullptr));
    if (!prop) return;

    role = std::string(
        reinterpret_cast<char*>(xcb_get_property_value(prop.get())),
        xcb_get_property_value_length(prop.get()));
}

static void fetch_class_and_instance(const xcb_window_t window_id,
                                     X11_window_property::WM_class& wm_class)
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::detail::conn(),
            xcb_get_property(X11::detail::conn(), false, window_id,
                             XCB_ATOM_WM_CLASS,
                             XCB_GET_PROPERTY_TYPE_ANY, 0, 128),
            nullptr));
    if (!prop) return;

    const char* prop_str = reinterpret_cast<char*>(
        xcb_get_property_value(prop.get()));

    const std::size_t prop_length  = xcb_get_property_value_length(prop.get());
    const std::size_t class_length = strnlen(prop_str, prop_length) + 1;

    wm_class.wclass   = std::string(prop_str, class_length);
    wm_class.instance = (class_length < prop_length)
                      ? std::string(prop_str + class_length,
                                    prop_length - class_length)
                      : "";
}

static void fetch_wm_hints(const xcb_window_t window_id, xcb_icccm_wm_hints_t& wm_hints)
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::detail::conn(),
            xcb_icccm_get_wm_hints(X11::detail::conn(), window_id),
            nullptr));
    if (xcb_get_property_value_length(prop.get())) return;
    xcb_icccm_get_wm_hints_from_reply(&wm_hints, prop.get());
}

static void fetch_protocols(const xcb_window_t window_id, std::vector<uint32_t>& protocols)
{
    xcb_icccm_get_wm_protocols_reply_t proto;
    if (!xcb_icccm_get_wm_protocols_reply(
            X11::detail::conn(),
            xcb_icccm_get_wm_protocols(X11::detail::conn(), window_id, X11::atom::WM_PROTOCOLS),
            &proto,
            nullptr))
        return;
    auto _ = memory::finally([&]{
        xcb_icccm_get_wm_protocols_reply_wipe(&proto);
    });
    protocols = { proto.atoms, proto.atoms + proto.atoms_len };
}

static void init_xprop(const uint32_t window_id, X11_window_property& xprop)
{
    fetch_name(window_id, xprop.name);
    fetch_type(window_id, xprop.type);
    fetch_role(window_id, xprop.role);
    fetch_class_and_instance(window_id, xprop.wm_class);
    fetch_wm_hints(window_id, xprop.wm_hints);
    fetch_protocols(window_id, xprop.protocols);
}

} // namespace window

// X11 Window implementations
Window_impl::Window_impl(const Window& window)
    : _window(window)
{
    // Get window events
    const uint32_t mask_values[] = { XCB_GRAVITY_NORTH_WEST, config::X11::CHILD_EVENT_MASK };
    window::change_attributes(_window.index(), XCB_CW_WIN_GRAVITY | XCB_CW_EVENT_MASK, std::span{mask_values});

    if (extension::xshape().is_supported) {
        xcb_shape_select_input(X11::detail::conn(), _window.index(), XCB_SHAPE_NOTIFY);
    }

    // Get window properties.
    window::detail::init_xprop(_window.index(), _xprop);
    _do_not_focus = !_xprop.wm_hints.input
                 && std::ranges::contains(_xprop.protocols, X11::atom::WM_TAKE_FOCUS);

    xcb_change_save_set(X11::detail::conn(), XCB_SET_MODE_INSERT, window.index());
    xcb_map_window(X11::detail::conn(), window.index());
}

void Window_impl::update_rect() noexcept
{
    if (_window.state() != Window::State::Normal) return;
    switch (_window.placement_mode()) {
    case Window::Placement_mode::Tiling:
        window::configure_rect(_window.index(), {
            { _window.rect().pos.x + (int)config::GAP_SIZE, _window.rect().pos.y + (int)config::GAP_SIZE },
            { _window.rect().size.x - 2*(int)config::GAP_SIZE, _window.rect().size.y - 2*(int)config::GAP_SIZE }
        });
        break;
    case Window::Placement_mode::Floating:
    case Window::Placement_mode::Sticky:
        window::configure_rect(_window.index(), _window.rect());
        break;
    }

}

void Window_impl::update_focus() noexcept
{
    if (_window.focused()) {
        if (_do_not_focus) {
            logger::debug("Window focus -> sending WM_TAKE_FOCUS to window: {:#x}", _window.index());
            window::send_take_focus(_window.index());
        } else {
            logger::debug("Window focus -> setting input focus to window : {:#x}", _window.index());
            window::set_input_focus(_window.index());
        }
        X11::ewmh::update_net_active_window(_window.index());
    } else {
        xcb_set_input_focus(X11::detail::conn(), XCB_INPUT_FOCUS_POINTER_ROOT, X11::detail::main_window_id(),
                            XCB_CURRENT_TIME);
        X11::ewmh::update_net_active_window(XCB_NONE);
    }
}

void Window_impl::update_state(Window::State wstate) noexcept
{
    switch (wstate) {
    case Window::State::Normal:
        ewmh::update_net_wm_state_hidden(_window.index(), false);
        xcb_map_window(X11::detail::conn(), _window.index());
        break;
    case Window::State::Minimized:
        ewmh::update_net_wm_state_hidden(_window.index(), true);
        event::ignore_unmap(_window.index());
        xcb_unmap_window(X11::detail::conn(), _window.index());
        break;
    case Window::State::Maximized:
        // Make the window fullscreen.
        break;
    default: std::unreachable();
    }
}

void Window_impl::kill() noexcept
{
    if (std::ranges::contains(_xprop.protocols, atom::WM_DELETE_WINDOW)) {
        const xcb_client_message_event_t event = {
            .response_type = XCB_CLIENT_MESSAGE,
            .format        = 32,
            .sequence      = 0,
            .window        = _window.index(),
            .type          = atom::WM_PROTOCOLS,
            .data          = { .data32 { atom::WM_DELETE_WINDOW, Timestamp::get() } }
        };
        xcb_send_event(detail::conn(), false, _window.index(), XCB_EVENT_MASK_NO_EVENT, (const char*)&event);
        detail::conn().flush();
    } else {
        xcb_destroy_window(detail::conn(), _window.index());
    }
}

Window_impl::~Window_impl() noexcept
{
    if (extension::xshape().is_supported) {
        xcb_shape_select_input(detail::conn(), _window.index(), XCB_NONE);
    }
    xcb_change_save_set(detail::conn(), XCB_SET_MODE_DELETE, _window.index());
}


// Utilities

namespace window {

auto get_attribute(const uint32_t window_id) noexcept
    -> memory::c_owner<xcb_get_window_attributes_reply_t>
{
    return memory::c_own<xcb_get_window_attributes_reply_t>(
        xcb_get_window_attributes_reply(
            X11::detail::conn(),
            xcb_get_window_attributes(X11::detail::conn(), window_id),
        nullptr));
}

auto get_geometry(const uint32_t window_id) noexcept
    -> memory::c_owner<xcb_get_geometry_reply_t>
{
    return memory::c_own<xcb_get_geometry_reply_t>(
        xcb_get_geometry_reply(
            X11::detail::conn(),
            xcb_get_geometry(X11::detail::conn(), window_id),
        nullptr));
}

void configure_rect(const uint32_t window_id, const Vector2D& rect) noexcept
{
    static constexpr uint16_t mask = XCB_CONFIG_WINDOW_X
                                   | XCB_CONFIG_WINDOW_Y
                                   | XCB_CONFIG_WINDOW_WIDTH
                                   | XCB_CONFIG_WINDOW_HEIGHT;

    const int values[] = {
        rect.pos.x,
        rect.pos.y,
        rect.size.x,
        rect.size.y
    };

    xcb_configure_window(X11::detail::conn(), window_id, mask, values);
}

void grab_keys(const uint32_t window_id, const State& state) noexcept
{
    for (const auto& [keybind, _] : state.bindings()) {
        uint8_t keycode = X11::keysym_to_keycode(state.conn(), keybind.keysym);
        logger::debug("Grab keys -> keysym: {}, keycode: {}", keybind.keysym, keycode);
        xcb_grab_key(X11::detail::conn(), 0, window_id, keybind.modifiers, keycode,
                     XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    }
}

void grab_buttons(const uint32_t window_id) noexcept
{
    // default buttons
    static constexpr auto buttons = {
        XCB_BUTTON_INDEX_1,
        XCB_BUTTON_INDEX_2,
        XCB_BUTTON_INDEX_3,
    };
    for (const auto b : buttons) {
        xcb_grab_button(X11::detail::conn(), 0, window_id,
                        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                        X11::detail::root_window_id(), XCB_NONE, b, XCB_BUTTON_MASK_ANY);
    }
}

static auto _fetch_all()
    -> std::pair<memory::c_owner<xcb_query_tree_reply_t>, std::span<xcb_window_t>>
{
    auto query = memory::c_own<xcb_query_tree_reply_t>(
        xcb_query_tree_reply(X11::detail::conn(),
            xcb_query_tree(X11::detail::conn(), X11::detail::root_window_id()), nullptr));
    return {
        std::move(query),
        std::span{xcb_query_tree_children(query.get()), (uint64_t)xcb_query_tree_children_length(query.get())}
    };
}

static auto _fetch_workspace(const uint32_t window_id) -> uint32_t
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::detail::conn(),
            xcb_get_property(X11::detail::conn(), false, window_id,
                             X11::atom::_NET_WM_DESKTOP,
                             XCB_GET_PROPERTY_TYPE_ANY, 0,
                             std::numeric_limits<uint32_t>::max()),
            nullptr));
    assert_runtime((bool)prop, "Cannot get workspace");
    if (xcb_get_property_value_length(prop.get()) == 0) return 0;
    return reinterpret_cast<uint32_t*>(xcb_get_property_value(prop.get()))[0];
}

static void _manage(const uint32_t window_id, State& state, Workspace& workspace, const bool is_starting_up)
{
    if (state.windows().contains(window_id)) {
        logger::debug("Can't manage window -> already managed");
        return;
    }

    auto attribute = get_attribute(window_id);
    if (attribute->override_redirect) {
        logger::debug("Can't manage window -> override_redirect");
        return;
    }
    if (is_starting_up && attribute->map_state == XCB_MAP_STATE_UNMAPPED) {
        logger::debug("Can't manage window -> state unmapped");
        return;
    }

    try {
        if (is_starting_up) {
            Window& window = state.windows().manage(window_id, Window::Display_type::X11);
            ::window::move_to_workspace(window, workspace);
        } else {
            state.manage_window(window_id, Window::Display_type::X11);
        }

        window::grab_keys(window_id, state);

    } catch (const std::bad_alloc&) {
        logger::error("Can't manage window -> Memory bad allocation");
        xcb_kill_client(state.conn(), window_id);
        return;
    }
}

void load_all(State& state)
{
    auto [_, window_ids] = window::_fetch_all();
    xcb_grab_server(X11::detail::conn());
    for (auto window_id : window_ids) {
        Workspace& workspace = state.get_or_create_workspace(_fetch_workspace(window_id));
        window::_manage(window_id, state, workspace, true);
    }
    xcb_ungrab_server(X11::detail::conn());
}

void manage(const uint32_t window_id, State& state)
{
    _manage(window_id, state, state.current_workspace(), false);
}

void send_take_focus(const uint32_t window_id) noexcept
{
    const xcb_client_message_event_t event = {
        .response_type = XCB_CLIENT_MESSAGE,
        .format        = 32,
        .sequence      = 0,
        .window        = window_id,
        .type          = X11::atom::WM_PROTOCOLS,
        .data          = { .data32 { X11::atom::WM_TAKE_FOCUS, Timestamp::get() } }
    };
    xcb_send_event(X11::detail::conn(), false, window_id, XCB_EVENT_MASK_NO_EVENT, (const char*)&event);
    X11::detail::conn().flush();
}

void set_input_focus(const uint32_t window_id) noexcept
{
    uint32_t mask_values[] = { config::X11::CHILD_EVENT_MASK & ~XCB_EVENT_MASK_FOCUS_CHANGE };
    window::change_attributes(window_id, XCB_CW_EVENT_MASK, std::span{mask_values});

    xcb_set_input_focus(X11::detail::conn(), XCB_INPUT_FOCUS_POINTER_ROOT, window_id, XCB_CURRENT_TIME);

    mask_values[0] = config::X11::CHILD_EVENT_MASK;
    window::change_attributes(window_id, XCB_CW_EVENT_MASK, std::span{mask_values});
}

} // namespace window
} // namespace X11
