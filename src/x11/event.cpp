#include "event.h"
#include "atom.h"
#include "extension.h"
#include "window.h"

#include "../config.h"
#include "../logger.h"
#include "../state.h"
#include "../server.h"

#include <ranges>
#include <xcb/xproto.h>
#include <xkbcommon/xkbcommon.h>
#define explicit _explicit
#include <xcb/xkb.h>
#undef explicit

// xmacro(key, name);
#define SUPPORTED_EVENTS \
xmacro(DESTROY_NOTIFY, destroy_notify) \
xmacro(UNMAP_NOTIFY, unmap_notify) \
xmacro(MAP_REQUEST, map_request) \
xmacro(ENTER_NOTIFY, enter_notify) \
xmacro(FOCUS_IN, focus_in) \
xmacro(FOCUS_OUT, focus_out) \
xmacro(BUTTON_PRESS, button_press) \
xmacro(BUTTON_RELEASE, button_release) \
xmacro(KEY_PRESS, key_press) \
xmacro(KEY_RELEASE, key_release) \
xmacro(MOTION_NOTIFY, motion_notify) \
xmacro(SELECTION_CLEAR, selection_clear)

#define SUPPORTED_XKB_EVENTS \
xmacro(NEW_KEYBOARD_NOTIFY, xkb_new_keyboard_notify) \
xmacro(MAP_NOTIFY, xkb_map_notify) \
xmacro(STATE_NOTIFY, xkb_state_notify)

namespace X11::event {

#define xmacro(key, name) static void _on_##name (State& state, const xcb_##name##_event_t& event);
    SUPPORTED_EVENTS
    SUPPORTED_XKB_EVENTS
#undef xmacro

void init(const X11::Connection& conn)
{
    try {
        const uint32_t mask[] = { config::X11::ROOT_EVENT_MASK };
        window::change_attributes_c(root_window_id(conn),
                                    XCB_CW_EVENT_MASK,
                                    std::span{mask});
    } catch(const std::runtime_error& err) {
        // Rethrow with different message
        throw std::runtime_error("Another WM is running (X Server)");
    }
    window::grab_buttons(root_window_id(conn));
}

static void _handle_xkb(State& state, const Event& event)
{
    switch (event.data->pad0) {
    #define xmacro(key, name)  \
    case XCB_XKB_##key: \
        _on_##name (state, event); \
        return;
    SUPPORTED_XKB_EVENTS
    #undef xmacro
    default:
        return;
    }
}

void handle(State& state, const Event& event)
{
    const int type = event.data->response_type & ~0x80;

    switch (type) {
    #define xmacro(key, name) \
    case XCB_##key: \
        _on_##name (state, event); \
        break;
    SUPPORTED_EVENTS
    #undef xmacro
    default:
        if (extension::xkb().is_supported && type == extension::xkb().base_event)
            _handle_xkb(state, event);
        else
            logger::debug("Event handler -> Unhandled event type: {}", type);
        break;
    }
}

static std::unordered_map<xcb_window_t, uint32_t> _ignored_unmap_ids;
void ignore_unmap(xcb_window_t window_id)
{
    _ignored_unmap_ids[window_id]++;
}

bool is_unmap_ignored(xcb_window_t window_id)
{
    if (_ignored_unmap_ids.contains(window_id)) {
        auto& count = _ignored_unmap_ids.at(window_id);
        --count;
        if (count == 0) _ignored_unmap_ids.erase(window_id);
        return true;
    }
    return false;
}

// Implementations for each event

void _on_destroy_notify(State& state, const xcb_destroy_notify_event_t& event)
{
    logger::debug("Destroy notify -> converting to unmap notify");
    const xcb_unmap_notify_event_t unmap {
        .response_type  = XCB_UNMAP_NOTIFY,
        .pad0           = event.pad0,
        .sequence       = event.sequence,
        .event          = event.event,
        .window         = event.window,
        .from_configure = 0,
        .pad1           = {}
    };
    _on_unmap_notify(state, unmap);
}

void _on_unmap_notify(State& state, const xcb_unmap_notify_event_t& event)
{
    auto& win_mgr = state.windows();

    if (win_mgr.contains(event.window)) {
        logger::debug("Unmap notify -> unmapping window: {:#x}", event.window);
        if (is_unmap_ignored(event.window)) return;
        window::unmanage(event.window, state);
        xcb_delete_property(state.conn(), event.window, atom::_NET_WM_DESKTOP);
        xcb_delete_property(state.conn(), event.window, atom::_NET_WM_STATE);
    } else {
        logger::debug("Unmap notify -> ignoring unmanaged window: {:#x}", event.window);
        return;
    }
}

void _on_map_request(State& state, const xcb_map_request_event_t& event)
{
    auto& win_mgr = state.windows();

    if (const auto& winref = win_mgr[event.window]) {
        auto& window    = winref->get();
        auto& workspace = window.root<Workspace>();
        if (workspace == state.current_workspace()) {
            logger::debug("Map request -> remapping managed window: {:#x}", window.index());
            xcb_map_window(state.conn(), window.index());
            ::window::focus_window(workspace.window_list(), window);
        }
    } else {
        window::manage(event.window, state);
        ::window::focus_last(state.current_workspace().window_list());
    }
}

void _on_enter_notify(State& state, const xcb_enter_notify_event_t& event)
{
    logger::debug("Enter notify -> window: {:#x}", event.event);
    if (event.mode != XCB_NOTIFY_MODE_NORMAL || event.detail == XCB_NOTIFY_DETAIL_INFERIOR) {
        logger::debug("Enter notify -> ignoring undesired notify");
        return;
    }
    if (const auto& winref = state.windows()[event.event]) {
        logger::debug("Enter notify -> window is managed");
        ::window::try_focus_window(winref->get());
    }
}

void _on_focus_in(State& state, const xcb_focus_in_event_t& event)
{
    if (event.mode == XCB_NOTIFY_MODE_GRAB || event.mode == XCB_NOTIFY_MODE_UNGRAB) {
        logger::debug("Focus in -> ignoring keyboard grab focus notify");
        return;
    }

    if (event.detail == XCB_NOTIFY_DETAIL_POINTER) {
        logger::debug("Focus in -> ignoring detail pointer notify");
        return;
    }

    auto& win_mgr = state.windows();
    // Update focus if root window received focus in
    if (event.event == root_window_id(state.conn())) {
        logger::debug("Focus in -> got root window id");
        ::window::focus_last(state.current_workspace().window_list());
    } else if (const auto& winref = win_mgr[event.event]) {
        logger::debug("Focus in -> trying to focus window id: {:#x}", winref->get().index());
        ::window::try_focus_window(winref->get());
    }
}

// Only log for debug
void _on_focus_out(State&, const xcb_focus_out_event_t& event)
{
#ifndef NDEBUG
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
    logger::debug("Focus out -> window: {:#x}, detail: {}, mode: {}",
                  event.event, detail, mode);
#endif
}

void _on_button_press(State& state, const xcb_button_press_event_t& event)
{
    logger::debug("Button press -> x: {}, y: {}, window: {:#x}", event.event_x, event.event_y, event.event);
    if (event.child != XCB_NONE) {
        if (const auto& winref = state.windows()[event.child]) {
            logger::debug("Button press -> found child window: {:#x}", event.child);
            ::window::try_focus_window(winref->get());
        }
    }
    xcb_allow_events(state.conn(), XCB_ALLOW_REPLAY_POINTER, event.time);
}

void _on_button_release(State&, const xcb_button_release_event_t& event)
{
    logger::debug("Button release -> x: {}, y: {}, window: {:#x}", event.event_x, event.event_y, event.event);
}

void _on_key_press(State& state, const xcb_key_press_event_t& event)
{
    Keybind keybind = XKB::create_keybind(event.detail, event.state);
#ifndef NDEBUG
    {
        char keysym_name[32];
        xkb_keysym_get_name(keybind.keysym, keysym_name, sizeof(keysym_name));
        logger::debug("Key press -> key: {}, mod: {}", keysym_name, keybind.modifiers);
    }
#endif
    if (const auto& bindref = state.bindings()[keybind]) {
        logger::debug("Key press -> found binding for keybind");
        bindref->get().execute(state);
    }
}

void _on_key_release(State&, const xcb_key_release_event_t& event)
{
#ifndef NDEBUG
    Keybind keybind = XKB::create_keybind(event.detail, event.state);
    char keysym_name[32];
    xkb_keysym_get_name(keybind.keysym, keysym_name, sizeof(keysym_name));
    logger::debug("Key release -> key: {}, mod: {}", keysym_name, keybind.modifiers);
#endif
}

void _on_motion_notify(State&, const xcb_motion_notify_event_t& event)
{
    Timestamp::update(event.time);
}

void _on_selection_clear(State&, const xcb_selection_clear_event_t& event)
{
    if (event.selection != X11::atom::WM_SN) {
        logger::debug("Selection clear -> unknown selection: {}", event.selection);
        return;
    }
    ::Server::instance().stop();
}

void _on_xkb_new_keyboard_notify(State&, const xcb_xkb_new_keyboard_notify_event_t& event)
{
    if (event.changed & XCB_XKB_NKN_DETAIL_KEYCODES)
        XKB::instance().update_keymap();
}

void _on_xkb_map_notify(State&, const xcb_xkb_map_notify_event_t&)
{
    XKB::instance().update_keymap();
}

void _on_xkb_state_notify(State& state, const xcb_xkb_state_notify_event_t& event)
{
    xkb_state_update_mask(XKB::instance().state(),
                          event.baseMods,
                          event.latchedMods,
                          event.lockedMods,
                          event.baseGroup,
                          event.latchedGroup,
                          event.lockedGroup);
    if (event.changed & XCB_XKB_STATE_PART_GROUP_STATE) {
        logger::debug("XKB state notify -> Group state changed");
        xcb_ungrab_key(state.conn(), XCB_GRAB_ANY, root_window_id(state.conn()), XCB_MOD_MASK_ANY);
        window::grab_keys(root_window_id(state.conn()), state);
    }
}

} // namespace X11::event


