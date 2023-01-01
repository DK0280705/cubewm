#include "xwrap.h"
#include "atoms.h"
#include "connection.h"
#include "error.h"
#include "logger.h"
#include "window.h"
#include <limits>
#include <memory>
#include <xcb/xcb.h>

extern "C" {
#include <xcb/xproto.h>
}

namespace XWrap
{
// Bogus code, i need to init it first or boom segfault
static const Connection* conn = nullptr; // NOLINT

void init(const Connection& conn)
{
    if (XWrap::conn) throw Init_error("XWrap");
    XWrap::conn = &conn;
}

template<class T, std::size_t N>
inline static uint32_t change_property(const CP           mode,
                                       const xcb_window_t window,
                                       const xcb_atom_t   property,
                                       const xcb_atom_t   type,
                                       std::span<T, N>    data)
{
    // i dunno, let the compiler guess
    constexpr int format = [] {
        if constexpr (std::is_pointer<T>()) return sizeof(T);
        else return sizeof(T) * 8;
    }();
    return xcb_change_property_checked(*conn,
                                       static_cast<unsigned char>(mode),
                                       window,
                                       property,
                                       type,
                                       format,
                                       data.size(),
                                       data.data())
        .sequence;
}

uint32_t change_atom_property(const xcb_window_t        window,
                              const xcb_atom_t          property,
                              std::span<const uint32_t> data,
                              const CP                  mode)
{
    return change_property(mode, window, property, XCB_ATOM_ATOM, data);
}

uint32_t change_utf8string_property(const xcb_window_t     window,
                                    const xcb_atom_t       property,
                                    std::span<const char*> data,
                                    const CP               mode)
{
    return change_property(mode, window, property, Atom::UTF8_STRING, data);
}

uint32_t change_string_property(const xcb_window_t    window,
                                const xcb_atom_t      property,
                                std::span<const char> data,
                                const CP              mode)
{
    return change_property(mode, window, property, XCB_ATOM_STRING, data);
}

uint32_t change_window_property(const xcb_window_t        window,
                                const xcb_atom_t          property,
                                std::span<const uint32_t> data,
                                const CP                  mode)
{
    return change_property(mode, window, property, XCB_ATOM_WINDOW, data);
}

uint32_t change_cardinal_property(const xcb_window_t        window,
                                  const xcb_atom_t          property,
                                  std::span<const uint32_t> data,
                                  const CP                  mode)
{
    return change_property(mode, window, property, XCB_ATOM_CARDINAL, data);
}

uint32_t change_window_attributes(const xcb_window_t        window,
                                  const uint32_t            value_mask,
                                  std::span<const uint32_t> data)
{
    return xcb_change_window_attributes_checked(*conn,
                                                window,
                                                value_mask,
                                                data.data())
        .sequence;
}

auto get_window_attributes(const xcb_window_t window)
    -> std::unique_ptr<xcb_get_window_attributes_reply_t, void (*)(void*)>
{
    auto* reply = xcb_get_window_attributes_reply(
        *conn, xcb_get_window_attributes(*conn, window), 0);
    return std::unique_ptr<xcb_get_window_attributes_reply_t, void (*)(void*)>(
        reply, free);
}

std::vector<xcb_window_t> get_windows()
{
    auto query = std::unique_ptr<xcb_query_tree_reply_t, void (*)(void*)>(
        xcb_query_tree_reply(*conn,
                             xcb_query_tree(*conn, conn->screen()->root),
                             0),
        free);
    xcb_window_t* windows = xcb_query_tree_children(query.get());
    return std::vector<xcb_window_t>(windows,
                                     windows + xcb_query_tree_children_length(
                                                   query.get()));
}

xcb_atom_t get_window_type(const xcb_window_t window)
{
    auto prop = std::unique_ptr<xcb_get_property_reply_t, void (*)(void*)>(
        xcb_get_property_reply(
            *conn,
            xcb_get_property(*conn,
                             false,
                             window,
                             Atom::_NET_WM_WINDOW_TYPE,
                             XCB_GET_PROPERTY_TYPE_ANY,
                             0,
                             std::numeric_limits<uint32_t>::max()),
            0),
        free);
    if (!prop) return XCB_NONE;

    // Return the very first supported atom
    const xcb_atom_t* atoms = reinterpret_cast<xcb_atom_t*>(
        xcb_get_property_value(prop.get()));
    for (int i = 0;
         i < xcb_get_property_value_length(prop.get()) / (prop->format / 8);
         i++)
        if (atoms[i] == Atom::_NET_WM_WINDOW_TYPE_NORMAL ||
            atoms[i] == Atom::_NET_WM_WINDOW_TYPE_DIALOG ||
            atoms[i] == Atom::_NET_WM_WINDOW_TYPE_UTILITY ||
            atoms[i] == Atom::_NET_WM_WINDOW_TYPE_TOOLBAR ||
            atoms[i] == Atom::_NET_WM_WINDOW_TYPE_SPLASH ||
            atoms[i] == Atom::_NET_WM_WINDOW_TYPE_MENU ||
            atoms[i] == Atom::_NET_WM_WINDOW_TYPE_DROPDOWN_MENU ||
            atoms[i] == Atom::_NET_WM_WINDOW_TYPE_POPUP_MENU ||
            atoms[i] == Atom::_NET_WM_WINDOW_TYPE_TOOLTIP ||
            atoms[i] == Atom::_NET_WM_WINDOW_TYPE_NOTIFICATION)
            return atoms[i];
    return XCB_NONE;
}

std::string get_window_name(const xcb_window_t window)
{
    auto prop = std::unique_ptr<xcb_get_property_reply_t, void (*)(void*)>(
        xcb_get_property_reply(*conn,
                               xcb_get_property(*conn,
                                                false,
                                                window,
                                                Atom::_NET_WM_NAME,
                                                XCB_GET_PROPERTY_TYPE_ANY,
                                                0,
                                                128),
                               0),
        free);
    if (!prop) return "";

    return std::string(reinterpret_cast<char*>(
                           xcb_get_property_value(prop.get())),
                       xcb_get_property_value_length(prop.get()));
}

std::string get_window_role(const xcb_window_t window)
{
    auto prop = std::unique_ptr<xcb_get_property_reply_t, void (*)(void*)>(
        xcb_get_property_reply(*conn,
                               xcb_get_property(*conn,
                                                false,
                                                window,
                                                Atom::WM_WINDOW_ROLE,
                                                XCB_GET_PROPERTY_TYPE_ANY,
                                                0,
                                                128),
                               0),
        free);
    if (!prop) return "";

    return std::string(reinterpret_cast<char*>(
                           xcb_get_property_value(prop.get())),
                       xcb_get_property_value_length(prop.get()));
}

uint32_t get_window_workspace(const xcb_window_t window)
{
    auto prop = std::unique_ptr<xcb_get_property_reply_t, void (*)(void*)>(
        xcb_get_property_reply(
            *conn,
            xcb_get_property(*conn,
                             false,
                             window,
                             Atom::_NET_WM_DESKTOP,
                             XCB_GET_PROPERTY_TYPE_ANY,
                             0,
                             std::numeric_limits<uint32_t>::max()),
            0),
        free);
    return reinterpret_cast<uint32_t*>(xcb_get_property_value(prop.get()))[0];
}

auto get_window_class(const xcb_window_t window)
    -> std::pair<std::string, std::string>
{
    auto prop = std::unique_ptr<xcb_get_property_reply_t, void (*)(void*)>(
        xcb_get_property_reply(*conn,
                               xcb_get_property(*conn,
                                                false,
                                                window,
                                                XCB_ATOM_WM_CLASS,
                                                XCB_GET_PROPERTY_TYPE_ANY,
                                                0,
                                                128),
                               0),
        free);
    const char* prop_str = reinterpret_cast<char*>(
        xcb_get_property_value(prop.get()));
    const std::size_t length      = xcb_get_property_value_length(prop.get());
    const std::size_t class_index = strnlen(prop_str, length) + 1;
    return std::make_pair(std::string(prop_str, class_index),
                          (class_index < length)
                              ? std::string(prop_str + class_index,
                                            length - class_index)
                              : "");
}

void send_event(const xcb_window_t window,
                const uint32_t     event_mask,
                const char*        data)
{
    xcb_send_event(*conn, 0, window, event_mask, data);
}

bool check_error(const uint32_t sequence)
{
    auto reply = std::unique_ptr<xcb_generic_error_t, void (*)(void*)>(
        xcb_request_check(*conn, {sequence}), free);
    return !reply;
}

void configure_window(const Window& win)
{
    constexpr static uint16_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                                     XCB_CONFIG_WINDOW_WIDTH |
                                     XCB_CONFIG_WINDOW_HEIGHT;

    const uint32_t values[] = {win.rect.x,
                               win.rect.y,
                               win.rect.width,
                               win.rect.height};

    Log::debug("Window {} size -> x: {}, y: {}, width: {}, height: {}",
               win.id,
               win.rect.x,
               win.rect.y,
               win.rect.width,
               win.rect.height);

    xcb_configure_window(*conn, win.id, mask, values); // NOLINT
}

} // namespace XWrap
