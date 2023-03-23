#include "../logger.h"
#include "../state.h"
#include "../connection.h"
#include "atom.h"
#include "constant.h"
#include "extension.h"
#include "window.h"
#include "event.h"
#include <csignal>
#include "x11.h"
#include <unordered_set>
#include <algorithm>
#include <xkbcommon/xkbcommon.h>
#define explicit _explicit
#include <xcb/xkb.h>
#undef explicit
#include <xcb/xproto.h>

// xmacro(key, name);
#define SUPPORTED_EVENTS \
xmacro(MAP_REQUEST, map_request) \
xmacro(UNMAP_NOTIFY, unmap_notify) \
xmacro(DESTROY_NOTIFY, destroy_notify) \
xmacro(FOCUS_IN, focus_in) \
xmacro(FOCUS_OUT, focus_out) \
xmacro(BUTTON_PRESS, button_press) \
xmacro(BUTTON_RELEASE, button_release) \
xmacro(SELECTION_CLEAR, selection_clear)

static void _grab_default_buttons(const xcb_window_t window_id)
{
    static constexpr auto buttons = {
        XCB_BUTTON_INDEX_1,
        XCB_BUTTON_INDEX_2,
        XCB_BUTTON_INDEX_3,
    };
    xcb_grab_server(X11::_conn());
    for (const auto b : buttons) {
        xcb_grab_button(X11::_conn(), 0, window_id,
                        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
                        X11::_conn().xscreen()->root, XCB_NONE, b, XCB_BUTTON_MASK_ANY);
    }
    xcb_ungrab_server(X11::_conn());
}

namespace X11::event {

static std::unordered_set<uint32_t> _ignored_sequences;
static State* _state;

#define xmacro(key, name) static void _on_##name (const xcb_##name##_event_t& event);
    SUPPORTED_EVENTS
    xmacro(, xkb_state_notify)
#undef xmacro

void init(State& state)
{
    _state = &state;
    try {
        window::change_attributes_c(state.conn().xscreen()->root,
                                    XCB_CW_EVENT_MASK,
                                    {{constant::ROOT_EVENT_MASK}});
    } catch(const std::runtime_error& err) {
        // Rethrow with different message
        throw std::runtime_error("Another WM is running (X Server)");
    }

}

void handle(const Event& event)
{
    if (_ignored_sequences.contains(event.data->sequence)) {
        logger::debug("ignoring sequence {}", event.data->sequence);
        return;
    }
    const int type = event.data->response_type & ~0x80;
    if (extension::xkb.supported() && type == extension::xkb.event_base()) {
        switch(event.data->pad0) {
        case XCB_XKB_STATE_NOTIFY:
            logger::debug("XKB_STATE_NOTIFY event");
            return _on_xkb_state_notify(event);
        default:
            return;
        }
    }

    switch (type){
    #define xmacro(key, name)         \
    case XCB_##key:                   \
        logger::debug(#key " event"); \
        _on_##name (event);           \
        break;
    SUPPORTED_EVENTS
    #undef xmacro
    default:
        logger::debug("Unhandled event type: {}", type);
        break;
    }
}

// Implementations for each event

void _on_destroy_notify(const xcb_destroy_notify_event_t& event)
{
    const xcb_unmap_notify_event_t unmap {
        .sequence = event.sequence,
        .event    = event.event,
        .window   = event.window
    };
    _on_unmap_notify(unmap);
}

void _on_unmap_notify(const xcb_unmap_notify_event_t& event)
{
    Manager<::Window>& win_mgr = _state->manager<::Window>();
    if (!win_mgr.is_managed(event.window)) {
        logger::debug("ignoring UnmapNotify event, window: {}", event.window);
        return;
    }
    ::Window* win = win_mgr.at(event.window);
    if (win->busy()) {
        win->busy(false);
        return;
    }
    auto& window_list = win->workspace()->window_list();
    window_list.remove(win);
    purge(win);
    xcb_change_save_set(_state->conn(), XCB_SET_MODE_DELETE, win->index());

    win_mgr.unmanage(event.window);
}

void _on_map_request(const xcb_map_request_event_t& event)
{
    _ignored_sequences.insert(event.sequence);
    Manager<::Window>& win_mgr = _state->manager<::Window>();

    if (win_mgr.is_managed(event.window)) {
        logger::debug("ignoring managed window: {}", event.window);
        return;
    }

    if (!window::manageable(event.window, false))
        return;

    ::Window* win = win_mgr.manage<X11::Window>(event.window);
    place_to(_state->current_workspace(), win);
    xcb_map_window(X11::_conn(), win->index());
    xcb_change_save_set(_state->conn(), XCB_SET_MODE_INSERT, win->index());

    // Set focus
    auto& window_list = win->workspace()->window_list();
    window_list.add(win);
    window_list.focus(std::prev(window_list.end(), 1));
    _grab_default_buttons(win->index());
}

void _on_focus_in(const xcb_focus_in_event_t& event)
{
    auto& win_mgr = _state->manager<::Window>();
    // Update focus if root window received focus in
    if (event.event == _state->conn().xscreen()->root) {
        auto* workspc = _state->current_workspace();
        logger::debug("Refocusing focused window");
        if (auto* f = workspc->window_list().current())
            f->focus();
    }

    if (!win_mgr.is_managed(event.event)) {
        logger::debug("Ignoring unmanaged window");
        return;
    }

    if (event.mode == XCB_NOTIFY_MODE_GRAB || event.mode == XCB_NOTIFY_MODE_UNGRAB) {
        logger::debug("Ignoring keyboard grab focus notify");
        return;
    }

    if (event.detail == XCB_NOTIFY_DETAIL_POINTER) {
        logger::debug("Ignoring detail pointer notify");
        return;
    }

    ::Window* win = win_mgr.at(event.event);
    auto& window_list = win->workspace()->window_list();
    if (win == window_list.current()) {
        logger::debug("Ignoring current focused");
        return;
    }
    window_list.focus(std::find(window_list.begin(), window_list.end(), win));
}

// Only log for debug
void _on_focus_out(const xcb_focus_out_event_t& event)
{
#ifndef NDEBUG
    auto& win_mgr = _state->manager<::Window>();
    std::string_view win_name = win_mgr.is_managed(event.event) ? win_mgr.at(event.event)->name() : "";
    std::string_view detail = [&] {
        switch (event.detail) {
        case XCB_NOTIFY_DETAIL_ANCESTOR:
            return "Ancestor";
            break;
        case XCB_NOTIFY_DETAIL_VIRTUAL:
            return "Virtual";
            break;
        case XCB_NOTIFY_DETAIL_INFERIOR:
            return "Inferior";
            break;
        case XCB_NOTIFY_DETAIL_NONLINEAR:
            return "Nonlinear";
            break;
        case XCB_NOTIFY_DETAIL_NONLINEAR_VIRTUAL:
            return "NonlinearVirtual";
            break;
        case XCB_NOTIFY_DETAIL_POINTER:
            return "Pointer";
            break;
        case XCB_NOTIFY_DETAIL_POINTER_ROOT:
            return "PointerRoot";
            break;
        case XCB_NOTIFY_DETAIL_NONE:
            return "None";
            break;
        default:
            return "<undefined>";
            break;
        }
    }();
    std::string_view mode = [&] {
        switch (event.mode) {
        case XCB_NOTIFY_MODE_NORMAL:
            return "Normal";
            break;
        case XCB_NOTIFY_MODE_GRAB:
            return "Grab";
            break;
        case XCB_NOTIFY_MODE_UNGRAB:
            return "Ungrab";
            break;
        case XCB_NOTIFY_MODE_WHILE_GRABBED:
            return "WhileGrabbed";
            break;
        default:
            return "<undefined>";
            break;
        }
    }();
    logger::debug("Focus lost -> window: {:#x}, name: {}, detail: {}, mode: {}",
                  event.event, win_name, detail, mode);
#endif
}

void _on_button_press(const xcb_button_press_event_t& event)
{
    logger::debug("Button press on -> x: {}, y: {}, window: {:#x}", event.event_x, event.event_y, event.event);
    if (_state->manager<::Window>().is_managed(event.event)) {
        auto* window = _state->manager<::Window>().at(event.event);
        auto& window_list = window->workspace()->window_list();
        xcb_allow_events(X11::_conn(), XCB_ALLOW_REPLAY_POINTER, event.time);
        if (window_list.current() == window) return;
        window_list.focus(std::find(window_list.begin(), window_list.end(), window));
    }
}

void _on_button_release(const xcb_button_release_event_t& event)
{
    logger::debug("Button release on -> x: {}, y: {}, window: {:#x}", event.event_x, event.event_y, event.event);
    logger::debug("Window is {}managed", _state->manager<::Window>().is_managed(event.event) ? "" : "not ");
}

void _on_selection_clear(const xcb_selection_clear_event_t& event)
{
    if (event.selection != X11::atom::WM_SN) {
        logger::debug("Selection clear for unknown selection: {}", event.selection);
        return;
    }
    // Suicide signal
    _state->server().stop();
}

static void _grab_default_keys()
{
    Keybind& keybind = _state->keybind();
    static const std::array<uint32_t, 4> keys = {
        keybind.keycode_from_keysym(XKB_KEY_H),
        keybind.keycode_from_keysym(XKB_KEY_J),
        keybind.keycode_from_keysym(XKB_KEY_K),
        keybind.keycode_from_keysym(XKB_KEY_L)
    };
    for (const auto key : keys) {
        xcb_grab_key(X11::_conn(), 0, X11::_conn().xscreen()->root,
                     XCB_MOD_MASK_4, key, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC);
    }
}

void _on_xkb_state_notify(const xcb_xkb_state_notify_event_t& event)
{
    logger::debug("State group: {}", event.group);
    _state->keybind().current_group(event.group);
    xcb_ungrab_key(X11::_conn(), XCB_GRAB_ANY, X11::_conn().xscreen()->root, XCB_MOD_MASK_ANY);
    _grab_default_keys();
}

} // namespace X11::event


