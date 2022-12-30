#include "xwrap.h"
#include "atoms.h"
#include "connection.h"
#include "error.h"
#include "logger.h"
#include "window.h"

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
inline static uint32_t change_property(const CP_mode      mode,
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
                                       mode,
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
                              const CP_mode             mode)
{
    return change_property(mode, window, property, XCB_ATOM_ATOM, data);
}

uint32_t change_utf8string_property(const xcb_window_t     window,
                                    const xcb_atom_t       property,
                                    std::span<const char*> data,
                                    const CP_mode          mode)
{
    return change_property(mode, window, property, Atom::UTF8_STRING, data);
}

uint32_t change_string_property(const xcb_window_t    window,
                                const xcb_atom_t      property,
                                std::span<const char> data,
                                const CP_mode         mode)
{
    return change_property(mode, window, property, XCB_ATOM_STRING, data);
}

uint32_t change_window_property(const xcb_window_t        window,
                                const xcb_atom_t          property,
                                std::span<const uint32_t> data,
                                const CP_mode             mode)
{
    return change_property(mode, window, property, XCB_ATOM_WINDOW, data);
}

uint32_t change_cardinal_property(const xcb_window_t        window,
                                  const xcb_atom_t          property,
                                  std::span<const uint32_t> data,
                                  const CP_mode             mode)
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

void send_event(const xcb_window_t window,
                const uint32_t     event_mask,
                const char*        data)
{
    xcb_send_event(*conn, 0, window, event_mask, data);
}

bool check_error(const uint32_t sequence)
{
    auto* reply  = xcb_request_check(*conn, {sequence});
    bool  result = !reply;
    free(reply);
    return result;
}

void configure_window(const Window& win)
{
    constexpr static uint16_t mask = XCB_CONFIG_WINDOW_X
                                   | XCB_CONFIG_WINDOW_Y
                                   | XCB_CONFIG_WINDOW_WIDTH
                                   | XCB_CONFIG_WINDOW_HEIGHT;

    const uint32_t values[] = {
        win.rect.x,
        win.rect.y,
        win.rect.width,
        win.rect.height
    };
    
    Log::debug("Window {} size -> x: {}, y: {}, width: {}, height: {}",
               win.id(), win.rect.x, win.rect.y, win.rect.width, win.rect.height);

    xcb_configure_window(*conn, win.id(), mask, values); // NOLINT
}

} // namespace XWrap
