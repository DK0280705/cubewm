#include "../logger.h"
#include "../state.h"
#include "../connection.h"
#include "atom.h"
#include "constant.h"
#include "extension.h"
#include "window.h"
#include "event.h"
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
        const uint32_t mask[] = { constant::ROOT_EVENT_MASK };
        window::change_attributes_c(conn.xscreen()->root,
                                    XCB_CW_EVENT_MASK,
                                    std::span{mask});
    } catch(const std::runtime_error& err) {
        // Rethrow with different message
        throw std::runtime_error("Another WM is running (X Server)");
    }
    window::grab_buttons(conn.xscreen()->root);
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
        if (extension::xkb.supported() && type == extension::xkb.event_base())
            _handle_xkb(state, event);
        else
            logger::debug("Event handler -> Unhandled event type: {}", type);
        break;
    }
}

// Implementations for each event

void _on_destroy_notify(State& state, const xcb_destroy_notify_event_t& event)
{
    const xcb_unmap_notify_event_t unmap {
        .sequence = event.sequence,
        .event    = event.event,
        .window   = event.window
    };
    _on_unmap_notify(state, unmap);
}

void _on_unmap_notify(State& state, const xcb_unmap_notify_event_t& event)
{
    Manager<::Window>& win_mgr = state.manager<::Window>();

    if (const auto& winref = win_mgr.get(event.window)) {
        ::Window& window = winref->get();
        if (window.busy()) {
            window.busy(false);
            return;
        }
        auto& window_list = window.root<Workspace>().window_list();
        window_list.remove(std::ranges::find(window_list, window));

        purge(window);
        xcb_change_save_set(state.conn(), XCB_SET_MODE_DELETE, window.index());

        win_mgr.unmanage(window.index());
    } else {
        logger::debug("Unmap notify -> ignoring unmanaged window: {:#x}", event.window);
        return;
    }
}

void _on_map_request(State& state, const xcb_map_request_event_t& event)
{
    Manager<::Window>& win_mgr = state.manager<::Window>();

    if (win_mgr.contains(event.window)) {
        logger::debug("Map request -> ignoring managed window: {:#x}", event.window);
        return;
    }

    if (!window::manageable(event.window, false))
        return;

    ::Window& window = win_mgr.manage<X11::Window>(event.window);
    window::grab_keys(state.keyboard(), window.index());
    place(window, state.current_workspace());
    xcb_map_window(state.conn(), window.index());
    xcb_change_save_set(state.conn(), XCB_SET_MODE_INSERT, window.index());

    // Set focus
    auto& window_list = window.root<Workspace>().window_list();
    window_list.add(window);
    window_list.focus(std::prev(window_list.end(), 1));
}

void _on_enter_notify(State& state, const xcb_enter_notify_event_t& event)
{
    logger::debug("Enter notify -> window: {:#x}", event.event);
    if (event.mode != XCB_NOTIFY_MODE_NORMAL || event.detail == XCB_NOTIFY_DETAIL_INFERIOR) {
        logger::debug("Enter notify -> ignoring undesired notify");
        return;
    }
    if (const auto& winref = state.manager<::Window>().get(event.event)) {
        ::Window& window = winref->get();
        logger::debug("Enter notify -> window is managed");
        auto window_list = window.root<Workspace>().window_list();
        window_list.focus(std::ranges::find(window_list, window));
    }
}

void _on_focus_in(State& state, const xcb_focus_in_event_t& event)
{
    auto& win_mgr = state.manager<::Window>();
    // Update focus if root window received focus in
    if (event.event == state.conn().xscreen()->root) {
        auto& workspace = state.current_workspace();
        logger::debug("Focus in -> refocusing focused window");
        if (const auto& f = workspace.window_list().current())
            f->get().focus();
    }

    if (event.mode == XCB_NOTIFY_MODE_GRAB || event.mode == XCB_NOTIFY_MODE_UNGRAB) {
        logger::debug("Focus in -> ignoring keyboard grab focus notify");
        return;
    }

    if (event.detail == XCB_NOTIFY_DETAIL_POINTER) {
        logger::debug("Focus in -> ignoring detail pointer notify");
        return;
    }

    if (const auto& winref = win_mgr.get(event.event)) {
        ::Window& window = winref->get();
        auto& window_list = window.root<Workspace>().window_list();
        if (window_list.current().has_value() and window != window_list.current()->get()) {
            window_list.focus(std::ranges::find(window_list, window));
        } else logger::debug("Focus in -> ignoring current focused");
    } else {
        logger::debug("Focus in -> ignoring unmanaged window");
    }
}

// Only log for debug
void _on_focus_out(State& state, const xcb_focus_out_event_t& event)
{
#ifndef NDEBUG
    auto& win_mgr = state.manager<::Window>();
    std::string_view win_name = win_mgr.contains(event.event) ? win_mgr.at(event.event).name() : "";
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

void _on_button_press(State& state, const xcb_button_press_event_t& event)
{
    logger::debug("Button press -> x: {}, y: {}, window: {:#x}", event.event_x, event.event_y, event.event);
    if (event.child != XCB_NONE) {
        if (const auto& winref = state.manager<::Window>().get(event.child)) {
            logger::debug("Button press -> found child window: {:#x}", event.child);
            auto& window = winref->get();
            auto& window_list = window.root<Workspace>().window_list();
            if (window_list.current().has_value() and window_list.current() != window)
                window_list.focus(std::ranges::find(window_list, window));
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
    xkb_keysym_t keysym = xkb_state_key_get_one_sym(state.keyboard().state(), event.detail);
    uint16_t modifiers = event.state & ~XCB_MOD_MASK_LOCK;

    char keysym_name[32];
    xkb_keysym_get_name(keysym, keysym_name, sizeof(keysym_name));
    logger::debug("Key press -> key: {}", keysym_name);

    Keybind keybind = {keysym, modifiers};
    if (const auto& bindref = state.keyboard().bindings().get(keybind)) {
        logger::debug("Key press -> found binding for keybind");
        bindref->get()(state);
    }
}

void _on_key_release(State& state, const xcb_key_release_event_t& event)
{
    xkb_keysym_t keysym = xkb_state_key_get_one_sym(state.keyboard().state(), event.detail);
    char keysym_name[32];
    xkb_keysym_get_name(keysym, keysym_name, sizeof(keysym_name));
    logger::debug("Key release -> key: {}", keysym_name);
}

void _on_motion_notify(State&, const xcb_motion_notify_event_t& event)
{
    State::timestamp().update(event.time);
}

void _on_selection_clear(State& state, const xcb_selection_clear_event_t& event)
{
    if (event.selection != X11::atom::WM_SN) {
        logger::debug("Selection clear -> unknown selection: {}", event.selection);
        return;
    }
    state.server().stop();
}

void _on_xkb_new_keyboard_notify(State& state, const xcb_xkb_new_keyboard_notify_event_t& event)
{
    if (event.changed & XCB_XKB_NKN_DETAIL_KEYCODES)
        state.keyboard().update_keymap();
}

void _on_xkb_map_notify(State& state, const xcb_xkb_map_notify_event_t&)
{
    state.keyboard().update_keymap();
}

void _on_xkb_state_notify(State& state, const xcb_xkb_state_notify_event_t& event)
{
    xkb_state_update_mask(state.keyboard().state(),
                          event.baseMods,
                          event.latchedMods,
                          event.lockedMods,
                          event.baseGroup,
                          event.latchedGroup,
                          event.lockedGroup);
    if (event.changed & XCB_XKB_STATE_PART_GROUP_STATE) {
        logger::debug("XKB state notify -> Group state changed");
        xcb_ungrab_key(state.conn(), XCB_GRAB_ANY, state.conn().xscreen()->root, XCB_MOD_MASK_ANY);
        window::grab_keys(state.keyboard(), state.conn().xscreen()->root);
    }
}

} // namespace X11::event


