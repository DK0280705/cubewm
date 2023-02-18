#include "x11.h"
#include "atom.h"
#include "constant.h"
#include "window.h"
#include "../connection.h"
#include "../state.h"
#include "../window_helper.h"
#include "../logger.h"
#include <limits>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

namespace X11 {

Window::Window(Managed_id id)
    : ::Window(id)
{
    uint32_t mask[] = { constant::CHILD_EVENT_MASK & ~XCB_EVENT_MASK_ENTER_WINDOW };
    window::change_attributes(id, XCB_CW_EVENT_MASK, mask);
    _fetch_name();
    _fetch_type();
    _fetch_role();
    _fetch_class();
}

void Window::update_rect(const Vector2D& rect)
{
    Container::update_rect(rect);
    constexpr static uint16_t mask = XCB_CONFIG_WINDOW_X
                                   | XCB_CONFIG_WINDOW_Y
                                   | XCB_CONFIG_WINDOW_WIDTH
                                   | XCB_CONFIG_WINDOW_HEIGHT;

    const int values[] = {
        rect.pos.x,
        rect.pos.y,
        rect.size.x,
        rect.size.y
    };

    xcb_configure_window(X11::_conn(), index(), mask, values); // NOLINT
}

void Window::focus()
{
    assert(!_focused);
    xcb_set_input_focus(X11::_conn(), XCB_INPUT_FOCUS_POINTER_ROOT, index(),
                        X11::_conn().timestamp());
    _focused = true;
}

void Window::unfocus()
{
    assert(_focused);
    xcb_set_input_focus(X11::_conn(), XCB_INPUT_FOCUS_POINTER_ROOT, XCB_NONE,
                        X11::_conn().timestamp());
    _focused = false;
}

void Window::_fetch_name()
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::_conn(),
            xcb_get_property(X11::_conn(), false,
                             index(), X11::atom::_NET_WM_NAME,
                             XCB_GET_PROPERTY_TYPE_ANY, 0, 128),
            nullptr));
    if (!prop) return;

    _name = std::string(
        reinterpret_cast<char*>(xcb_get_property_value(prop.get())),
        xcb_get_property_value_length(prop.get()));
    logger::debug("name: {}", _name);
}

void Window::_fetch_type()
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::_conn(),
            xcb_get_property(X11::_conn(), false, index(),
                             X11::atom::_NET_WM_WINDOW_TYPE,
                             XCB_GET_PROPERTY_TYPE_ANY, 0,
                             std::numeric_limits<uint32_t>::max()),
            nullptr));
    if (!prop) return;

    // Return the very first supported atom
    const xcb_atom_t* atoms = reinterpret_cast<xcb_atom_t*>(
        xcb_get_property_value(prop.get()));
    for (int i = 0;
         i < xcb_get_property_value_length(prop.get()) / sizeof(xcb_atom_t);
         i++)
        if (atoms[i] == X11::atom::_NET_WM_WINDOW_TYPE_NORMAL ||
            atoms[i] == X11::atom::_NET_WM_WINDOW_TYPE_DIALOG ||
            atoms[i] == X11::atom::_NET_WM_WINDOW_TYPE_UTILITY ||
            atoms[i] == X11::atom::_NET_WM_WINDOW_TYPE_TOOLBAR ||
            atoms[i] == X11::atom::_NET_WM_WINDOW_TYPE_SPLASH ||
            atoms[i] == X11::atom::_NET_WM_WINDOW_TYPE_MENU ||
            atoms[i] == X11::atom::_NET_WM_WINDOW_TYPE_DROPDOWN_MENU ||
            atoms[i] == X11::atom::_NET_WM_WINDOW_TYPE_POPUP_MENU ||
            atoms[i] == X11::atom::_NET_WM_WINDOW_TYPE_TOOLTIP ||
            atoms[i] == X11::atom::_NET_WM_WINDOW_TYPE_NOTIFICATION) {
            _type = atoms[i];
            logger::debug("type: {}", _type);
            return;
        }
}

void Window::_fetch_role()
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::_conn(),
            xcb_get_property(X11::_conn(), false, index(),
                             X11::atom::WM_WINDOW_ROLE,
                             XCB_GET_PROPERTY_TYPE_ANY, 0, 128),
            nullptr));
    if (!prop) return;

    _role = std::string(
        reinterpret_cast<char*>(xcb_get_property_value(prop.get())),
        xcb_get_property_value_length(prop.get()));
    logger::debug("role: {}", _role);
}

void Window::_fetch_class()
{
    auto prop = memory::c_own<xcb_get_property_reply_t>(
        xcb_get_property_reply(
            X11::_conn(),
            xcb_get_property(X11::_conn(), false, index(),
                             XCB_ATOM_WM_CLASS,
                             XCB_GET_PROPERTY_TYPE_ANY, 0, 128),
            nullptr));
    if (!prop) return;

    const char* prop_str = reinterpret_cast<char*>(
        xcb_get_property_value(prop.get()));

    if (!prop_str) throw std::runtime_error("Cannot get window class");
    
    const std::size_t length      = xcb_get_property_value_length(prop.get());
    const std::size_t class_index = strnlen(prop_str, length) + 1;

    _class    = std::string(prop_str, class_index);
    _instance = (class_index < length)
              ? std::string(prop_str + class_index, length - class_index)
              : "";
    logger::debug("class: {}, instance: {}", _class, _instance);
}


namespace window {

bool manageable(uint32_t window_id, const bool must_be_mapped)
{
    auto attr = get_attribute(window_id);
   
    if (attr->override_redirect) {
        logger::debug("Ignoring override_redirect");
        return false;
    }

    if (must_be_mapped && attr->map_state == XCB_MAP_STATE_UNMAPPED) {
        logger::debug("Ignoring unmapped window");
        return false;
    }

    auto geo = get_geometry(window_id);

    if (!geo) {
        logger::debug("Can't get window geometry");
        return false;
    }

    return true;

}

static auto all() -> std::pair<memory::c_owner<xcb_query_tree_reply_t>, std::span<xcb_window_t>>
{
    auto query = memory::c_own<xcb_query_tree_reply_t>(
        xcb_query_tree_reply(X11::_conn(),
            xcb_query_tree(X11::_conn(), X11::_conn().xscreen()->root), 0));
    xcb_window_t* windows = xcb_query_tree_children(query.get());
    return std::pair(
        std::move(query),
        std::span{windows, (uint64_t)xcb_query_tree_children_length(query.get())});
}

void load_all(State& state)
{
    Manager<::Window>& win_mgr = state.manager<::Window>();
    auto pair = all();
    for (const auto& w_id : pair.second) {
        if (win_mgr.is_managed(w_id)) continue;
        if (!manageable(w_id, true)) continue;
        ::Window* window = win_mgr.manage<X11::Window>(w_id);
        
        ::Workspace* ws = load_workspace(state, dynamic_cast<X11::Window*>(window));

        ws->accept(place(window));
        ws->focus_list().add(window);
        xcb_map_window(X11::_conn(), window->index());
    }
}

static uint32_t fetch_workspace(uint32_t window_id)
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


auto load_workspace(State& state, X11::Window* window) -> ::Workspace*
{
    const uint32_t ws_id          = fetch_workspace(window->index());
    Manager<::Workspace>& wor_mgr = state.manager<::Workspace>(); 

    return (wor_mgr.is_managed(ws_id)) ? wor_mgr.at(ws_id) : wor_mgr.manage(ws_id);
}

auto get_attribute(uint32_t window_id)
    -> memory::c_owner<xcb_get_window_attributes_reply_t>
{
    return memory::c_own<xcb_get_window_attributes_reply_t>(
        xcb_get_window_attributes_reply(
            X11::_conn(),
            xcb_get_window_attributes(X11::_conn(), window_id),
        0));
}

auto get_geometry(uint32_t window_id)
    -> memory::c_owner<xcb_get_geometry_reply_t>
{
    return memory::c_own<xcb_get_geometry_reply_t>(
        xcb_get_geometry_reply(
            X11::_conn(),
            xcb_get_geometry(X11::_conn(), window_id),
        0));
}

static void check_error(const xcb_void_cookie_t& cookie)
{
    auto reply = memory::c_own(xcb_request_check(X11::_conn(), cookie));
    assert_runtime(!reply, "Change property failed");
}

namespace detail {
void _cpc_impl(const window::prop mode,
               const uint32_t     wind,
               const uint8_t      prop,
               const uint8_t      type,
               const uint8_t      form,
               const uint32_t     size,
               const void*        data)
{
    check_error(xcb_change_property_checked(
        X11::_conn(), static_cast<uint8_t>(mode),
        wind, prop, type, form, size, data));
}
void _cp_impl(const window::prop mode,
              const uint32_t     wind,
              const uint8_t      prop,
              const uint8_t      type,
              const uint8_t      form,
              const uint32_t     size,
              const void*        data)
{
    xcb_change_property(
        X11::_conn(), static_cast<uint8_t>(mode),
        wind, prop, type, form, size, data);
}
} // namespace detail

void change_attributes(const uint32_t wind, const uint32_t mask, std::span<const uint32_t> data)
{
    xcb_change_window_attributes(X11::_conn(), wind, mask, data.data());
}

void change_attributes_c(const uint32_t wind, const uint32_t mask, std::span<const uint32_t> data)
{
    check_error(xcb_change_window_attributes_checked(X11::_conn(), wind, mask, data.data()));
}

} // namespace window
    
} // namespace X11
