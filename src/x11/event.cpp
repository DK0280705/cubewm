#include "../logger.h"
#include "../state.h"
#include "../connection.h"
#include "atom.h"
#include "constant.h"
#include "extension.h"
#include "window.h"
#include "event.h"
#include <algorithm>
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

static State* _state;

#define xmacro(key, name) static void _on_##name (const xcb_##name##_event_t& event);
    SUPPORTED_EVENTS
    SUPPORTED_XKB_EVENTS
#undef xmacro

void init(State& state)
{
    _state = &state;
    try {
        const uint32_t mask[] = { constant::ROOT_EVENT_MASK };
        window::change_attributes_c(state.conn().xscreen()->root,
                                    XCB_CW_EVENT_MASK,
                                    std::span{mask});
    } catch(const std::runtime_error& err) {
        // Rethrow with different message
        throw std::runtime_error("Another WM is running (X Server)");
    }
    window::grab_buttons(state.conn().xscreen()->root);
}

static void _handle_xkb(const Event& event)
{
    switch (event.data->pad0) {
    #define xmacro(key, name)  \
    case XCB_XKB_##key: \
        _on_##name (event); \
        return;
    SUPPORTED_XKB_EVENTS
    #undef xmacro
    default:
        return;
    }
}

void handle(const Event& event)
{
    const int type = event.data->response_type & ~0x80;

    switch (type) {
    #define xmacro(key, name) \
    case XCB_##key: \
        _on_##name (event); \
        break;
    SUPPORTED_EVENTS
    #undef xmacro
    default:
        if (extension::xkb.supported() && type == extension::xkb.event_base())
            _handle_xkb(event);
        else
            logger::debug("Event handler -> Unhandled event type: {}", type);
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
        logger::debug("Unmap notify -> ignoring unmanaged window: {:#x}", event.window);
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
    Manager<::Window>& win_mgr = _state->manager<::Window>();

    if (win_mgr.is_managed(event.window)) {
        logger::debug("Map request -> ignoring managed window: {:#x}", event.window);
        return;
    }

    if (!window::manageable(event.window, false))
        return;

    ::Window* win = win_mgr.manage<X11::Window>(event.window);
    place_to(_state->current_workspace(), win);
    xcb_map_window(_state->conn(), win->index());
    xcb_change_save_set(_state->conn(), XCB_SET_MODE_INSERT, win->index());

    // Set focus
    auto& window_list = win->workspace()->window_list();
    window_list.add(win);
    window_list.focus(std::prev(window_list.end(), 1));
}

void _on_enter_notify(const xcb_enter_notify_event_t& event)
{
    logger::debug("Enter notify -> window: {:#x}", event.event);
    if (event.mode != XCB_NOTIFY_MODE_NORMAL || event.detail == XCB_NOTIFY_DETAIL_INFERIOR) {
        logger::debug("Enter notify -> ignoring undesired notify");
        return;
    }
    if (_state->manager<::Window>().is_managed(event.event)) {
        auto* window = _state->manager<::Window>().at(event.event);
        logger::debug("Enter notify -> window is managed");
        auto window_list = window->workspace()->window_list();
        window_list.focus(std::find(window_list.begin(), window_list.end(), window));
    }
}

void _on_focus_in(const xcb_focus_in_event_t& event)
{
    auto& win_mgr = _state->manager<::Window>();
    // Update focus if root window received focus in
    if (event.event == _state->conn().xscreen()->root) {
        auto* workspc = _state->current_workspace();
        logger::debug("Focus in -> refocusing focused window");
        if (auto* f = workspc->window_list().current())
            f->focus();
    }

    if (!win_mgr.is_managed(event.event)) {
        logger::debug("Focus in -> ignoring unmanaged window");
        return;
    }

    if (event.mode == XCB_NOTIFY_MODE_GRAB || event.mode == XCB_NOTIFY_MODE_UNGRAB) {
        logger::debug("Focus in -> ignoring keyboard grab focus notify");
        return;
    }

    if (event.detail == XCB_NOTIFY_DETAIL_POINTER) {
        logger::debug("Focus in -> ignoring detail pointer notify");
        return;
    }

    ::Window* win = win_mgr.at(event.event);
    auto& window_list = win->workspace()->window_list();
    if (win == window_list.current()) {
        logger::debug("Focus in -> ignoring current focused");
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
    logger::debug("Focus out -> window: {:#x}, name: {}, detail: {}, mode: {}",
                  event.event, win_name, detail, mode);
#endif
}

void _on_button_press(const xcb_button_press_event_t& event)
{
    logger::debug("Button press -> x: {}, y: {}, window: {:#x}", event.event_x, event.event_y, event.event);
    if (event.child != XCB_NONE) {
        if (_state->manager<::Window>().is_managed(event.child)) {
            logger::debug("Button press -> found child window: {:#x}", event.child);
            auto* window = _state->manager<::Window>().at(event.child);
            auto& window_list = window->workspace()->window_list();
            if (window_list.current() != window)
                window_list.focus(std::find(window_list.begin(), window_list.end(), window));
        }
    }
    xcb_allow_events(_state->conn(), XCB_ALLOW_REPLAY_POINTER, event.time);
}

void _on_button_release(const xcb_button_release_event_t& event)
{
    logger::debug("Button release -> x: {}, y: {}, window: {:#x}", event.event_x, event.event_y, event.event);
}

void _on_key_press(const xcb_key_press_event_t& event)
{
    xkb_keysym_t keysym = xkb_state_key_get_one_sym(_state->keyboard().state(), event.detail);
    char keysym_name[32];
    xkb_keysym_get_name(keysym, keysym_name, sizeof(keysym_name));
    logger::debug("Key press -> key: {}", keysym_name);
}

void _on_key_release(const xcb_key_release_event_t& event)
{
    xkb_keysym_t keysym = xkb_state_key_get_one_sym(_state->keyboard().state(), event.detail);
    char keysym_name[32];
    xkb_keysym_get_name(keysym, keysym_name, sizeof(keysym_name));
    logger::debug("Key release -> key: {}", keysym_name);
}

void _on_motion_notify(const xcb_motion_notify_event_t& event)
{
    // Let's silent this thing.
}

void _on_selection_clear(const xcb_selection_clear_event_t& event)
{
    if (event.selection != X11::atom::WM_SN) {
        logger::debug("Selection clear -> unknown selection: {}", event.selection);
        return;
    }
    _state->server().stop();
}

void _on_xkb_new_keyboard_notify(const xcb_xkb_new_keyboard_notify_event_t& event)
{
    if (event.changed & XCB_XKB_NKN_DETAIL_KEYCODES)
        _state->keyboard().update_keymap();
}

void _on_xkb_map_notify(const xcb_xkb_map_notify_event_t& event)
{
    _state->keyboard().update_keymap();
}

void _on_xkb_state_notify(const xcb_xkb_state_notify_event_t& event)
{
    xkb_state_update_mask(_state->keyboard().state(),
                          event.baseMods,
                          event.latchedMods,
                          event.lockedMods,
                          event.baseGroup,
                          event.latchedGroup,
                          event.lockedGroup);
    if (event.changed & XCB_XKB_STATE_PART_GROUP_STATE) {
        xcb_ungrab_key(_state->conn(), XCB_GRAB_ANY, _state->conn().xscreen()->root, XCB_MOD_MASK_ANY);
        window::grab_keys(_state->keyboard(), _state->conn().xscreen()->root);
    }
}

} // namespace X11::event


