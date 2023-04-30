#include "../state.h"
#include "../logger.h"
#include "atom.h"
#include "constant.h"
#include "extension.h"
#include "keyboard.h"
#include "frame.h"
#include "window.h"
#include "x11.h"

#include <limits>
#include <algorithm>
#include <new>
#include <xcb/shape.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xproto.h>
#include <xkbcommon/xkbcommon-keysyms.h>

namespace X11 {

// Window information fetcher
namespace window {

static void _fetch_name(const xcb_window_t window_id, std::string& name)
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::_conn(),
            xcb_get_property(X11::_conn(), false,
                             window_id, X11::atom::_NET_WM_NAME,
                             XCB_GET_PROPERTY_TYPE_ANY, 0, 128),
            nullptr));
    if (!prop) return;

    name = std::string(
        reinterpret_cast<char*>(xcb_get_property_value(prop.get())),
        xcb_get_property_value_length(prop.get()));
    logger::debug("Window: {:#x} -> name: {}", window_id, name);
}

static void _fetch_type(const xcb_window_t window_id, xcb_atom_t& type)
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::_conn(),
            xcb_get_property(X11::_conn(), false, window_id,
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
            logger::debug("Window: {:#x} -> type: {}", window_id, type);
            return;
        }
}

static void _fetch_role(const xcb_window_t window_id, std::string& role)
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::_conn(),
            xcb_get_property(X11::_conn(), false, window_id,
                             X11::atom::WM_WINDOW_ROLE,
                             XCB_GET_PROPERTY_TYPE_ANY, 0, 128),
            nullptr));
    if (!prop) return;

    role = std::string(
        reinterpret_cast<char*>(xcb_get_property_value(prop.get())),
        xcb_get_property_value_length(prop.get()));
    logger::debug("Window: {:#x} -> role: {}", window_id, role);
}

static void _fetch_class_and_instance(const xcb_window_t window_id,
                                      std::string&       window_class,
                                      std::string&       window_instance)
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::_conn(),
            xcb_get_property(X11::_conn(), false, window_id,
                             XCB_ATOM_WM_CLASS,
                             XCB_GET_PROPERTY_TYPE_ANY, 0, 128),
            nullptr));
    if (!prop) return;

    const char* prop_str = reinterpret_cast<char*>(
        xcb_get_property_value(prop.get()));

    if (!prop_str) {
        logger::error("Window: {:#x} -> cannot get window class", window_id);
        return;
    }

    const std::size_t prop_length  = xcb_get_property_value_length(prop.get());
    const std::size_t class_length = strnlen(prop_str, prop_length) + 1;

    window_class    = std::string(prop_str, class_length);
    window_instance = (class_length < prop_length)
                    ? std::string(prop_str + class_length,
                                  prop_length - class_length)
                    : "";
    logger::debug("Window: {:#x} -> class: {}, instance: {}", window_id, window_class, window_instance);
}

static bool _is_alt_focus(const xcb_window_t window_id)
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::_conn(),
            xcb_icccm_get_wm_hints(X11::_conn(), window_id),
            nullptr));
    if (xcb_get_property_value_length(prop.get())) return true;

    xcb_icccm_wm_hints_t hints{};
    if (xcb_icccm_get_wm_hints_from_reply(&hints, prop.get())) return true;
    if (hints.flags & XCB_ICCCM_WM_HINT_INPUT) return hints.input;
    return true;
}
} // namespace window


// X11::Window implementations
Window::Window(Index id)
    : ::Window(id)
{
    // Give the window a frame
    _frame = new X11::Window_frame(*this);

    // Get window events
    const uint32_t mask_values[] = { XCB_GRAVITY_NORTH_WEST, constant::CHILD_EVENT_MASK };
    window::change_attributes(id, XCB_CW_WIN_GRAVITY | XCB_CW_EVENT_MASK, std::span{mask_values});
    if (extension::xshape.supported()) {
        xcb_shape_select_input(X11::_conn(), id, XCB_SHAPE_NOTIFY);
    }

    // Get information of the window
    window::_fetch_type(id, _type);
    window::_fetch_name(id, _name);
    window::_fetch_role(id, _role);
    window::_fetch_class_and_instance(id, _class, _instance);
    _alt_focus = !window::_is_alt_focus(id)
               && window::has_proto(id, X11::atom::WM_TAKE_FOCUS);
    logger::debug("Window: {:#x} -> alt focus: {}", index(), _alt_focus);

    // Flush the toilet
    xcb_flush(X11::_conn());
}

void Window::update_rect() noexcept
{
    _frame->rect(this->rect());
    window::configure_rect(index(), this->rect());
}

void Window::focus()
{
    if (_alt_focus) {
        logger::debug("Window focus -> sending WM_TAKE_FOCUS to window: {:#x}", index());
        window::send_take_focus(index());
    } else {
        logger::debug("Window focus -> setting input focus to window : {:#x}", index());
        window::set_input_focus(index());
    }
    _focused = true;
}

void Window::unfocus()
{
    xcb_set_input_focus(X11::_conn(), XCB_INPUT_FOCUS_POINTER_ROOT, X11::_main_window_id(),
                        XCB_CURRENT_TIME);
    _focused = false;
}


// Utilities

namespace window {

auto get_attribute(const uint32_t window_id) noexcept
    -> memory::c_owner<xcb_get_window_attributes_reply_t>
{
    return memory::c_own<xcb_get_window_attributes_reply_t>(
        xcb_get_window_attributes_reply(
            X11::_conn(),
            xcb_get_window_attributes(X11::_conn(), window_id),
        0));
}

auto get_geometry(const uint32_t window_id) noexcept
    -> memory::c_owner<xcb_get_geometry_reply_t>
{
    return memory::c_own<xcb_get_geometry_reply_t>(
        xcb_get_geometry_reply(
            X11::_conn(),
            xcb_get_geometry(X11::_conn(), window_id),
        0));
}

bool has_proto(const uint32_t window_id, const uint32_t atom) noexcept
{
    xcb_icccm_get_wm_protocols_reply_t protocols;
    if (!xcb_icccm_get_wm_protocols_reply(
            X11::_conn(),
            xcb_icccm_get_wm_protocols(X11::_conn(), window_id, X11::atom::WM_PROTOCOLS),
            &protocols,
            nullptr))
        return false;
    auto _ = finally([&]{
        xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
    });

    auto span = std::span{protocols.atoms, protocols.atoms_len};
    return std::find(span.begin(), span.end(), atom) != span.end();
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

    xcb_configure_window(X11::_conn(), window_id, mask, values);
}

void grab_keys(const uint32_t window_id, const ::Keyboard& keyboard) noexcept
{
    for (const auto& [keybind, _] : keyboard.bindings()) {
        uint8_t keycode = keysym_to_keycode(keybind.keysym);
        logger::debug("Grab keys -> keysym: {}, keycode: {}", keybind.keysym, keycode);
        xcb_grab_key(X11::_conn(), 0, window_id, keybind.modifiers, keycode,
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
        xcb_grab_button(X11::_conn(), 0, window_id,
                        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                        X11::_root_window_id(), XCB_NONE, b, XCB_BUTTON_MASK_ANY);
    }
}

static auto _fetch_all()
    -> std::pair<memory::c_owner<xcb_query_tree_reply_t>, std::span<xcb_window_t>>
{
    auto query = memory::c_own<xcb_query_tree_reply_t>(
        xcb_query_tree_reply(X11::_conn(),
            xcb_query_tree(X11::_conn(), X11::_conn().xscreen()->root), 0));
    xcb_window_t* window_ids = xcb_query_tree_children(query.get());
    return std::pair(
        std::move(query),
        std::span{window_ids, (uint64_t)xcb_query_tree_children_length(query.get())});
}

static uint32_t _fetch_workspace(const uint32_t window_id)
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::_conn(),
            xcb_get_property(X11::_conn(), false, window_id,
                             X11::atom::_NET_WM_DESKTOP,
                             XCB_GET_PROPERTY_TYPE_ANY, 0,
                             std::numeric_limits<uint32_t>::max()),
            0));
    assert_runtime(!!prop, "Cannot get workspace");
    if (xcb_get_property_value_length(prop.get()) == 0) return 0;
    return reinterpret_cast<uint32_t*>(xcb_get_property_value(prop.get()))[0];
}

static Workspace& _load_workspace(const uint32_t window_id, State& state)
{
    const auto ws_id = _fetch_workspace(window_id);
    auto& wor_mgr    = state.manager<Workspace>();
    return wor_mgr.contains(ws_id) ? wor_mgr.at(ws_id) : wor_mgr.manage(ws_id);
}

void load_all(State& state)
{
    auto [_, window_ids] = window::_fetch_all();
    xcb_grab_server(X11::_conn());
    for (auto window_id : window_ids) {
        Workspace& workspace = window::_load_workspace(window_id, state);
        window::manage(window_id, workspace, state, true);
    }
    xcb_ungrab_server(X11::_conn());
}

void manage(const uint32_t window_id, Workspace& workspace, State& state, const bool is_starting_up)
{
    if (state.manager<::Window>().contains(window_id)) {
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
        Window& window = state.manager<::Window>().manage<X11::Window>(window_id);

        window::grab_keys(window.index(), state.keyboard());
        move_to_workspace(window, workspace);
        xcb_change_save_set(state.conn(), XCB_SET_MODE_INSERT, window.index());
        xcb_map_window(state.conn(), window.index());

        add_window(workspace.window_list(), window, false);

    } catch (const std::bad_alloc&) {
        logger::error("Can't manage window -> Memory bad allocation");
        return;
    }
}

void send_take_focus(const uint32_t window_id) noexcept
{
    const xcb_client_message_event_t event = {
        .response_type = XCB_CLIENT_MESSAGE,
        .format        = 32,
        .sequence      = 0,
        .window        = window_id,
        .type          = X11::atom::WM_PROTOCOLS,
        .data          = { .data32 { X11::atom::WM_TAKE_FOCUS, State::timestamp() } }
    };
    xcb_send_event(X11::_conn(), false, window_id, XCB_EVENT_MASK_NO_EVENT, (const char*)&event);
}

void set_input_focus(const uint32_t window_id) noexcept
{
    uint32_t mask_values[] = { constant::CHILD_EVENT_MASK & ~XCB_EVENT_MASK_FOCUS_CHANGE };
    window::change_attributes(window_id, XCB_CW_EVENT_MASK, std::span{mask_values});

    xcb_set_input_focus(X11::_conn(), XCB_INPUT_FOCUS_POINTER_ROOT, window_id, XCB_CURRENT_TIME);

    mask_values[0] = constant::CHILD_EVENT_MASK;
    window::change_attributes(window_id, XCB_CW_EVENT_MASK, std::span{mask_values});
}

} // namespace window
} // namespace X11
