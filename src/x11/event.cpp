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
xmacro(MAP_REQUEST, map_request) \
xmacro(UNMAP_NOTIFY, unmap_notify) \
xmacro(DESTROY_NOTIFY, destroy_notify) \
xmacro(FOCUS_IN, focus_in) \
xmacro(FOCUS_OUT, focus_out) \
xmacro(BUTTON_PRESS, button_press) \
xmacro(BUTTON_RELEASE, button_release) \
xmacro(KEY_PRESS, key_press) \
xmacro(KEY_RELEASE, key_release) \
xmacro(SELECTION_CLEAR, selection_clear)

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
        const uint32_t mask[] = { constant::ROOT_EVENT_MASK };
        window::change_attributes_c(state.conn().xscreen()->root,
                                    XCB_CW_EVENT_MASK,
                                    std::span{mask});
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
    xcb_map_window(_state->conn(), win->index());
    xcb_change_save_set(_state->conn(), XCB_SET_MODE_INSERT, win->index());

    // Set focus
    auto& window_list = win->workspace()->window_list();
    window_list.add(win);
    window_list.focus(std::prev(window_list.end(), 1));
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
        xcb_allow_events(_state->conn(), XCB_ALLOW_REPLAY_POINTER, event.time);
        if (window_list.current() != window)
            window_list.focus(std::find(window_list.begin(), window_list.end(), window));
    }
}

void _on_button_release(const xcb_button_release_event_t& event)
{
    logger::debug("Button release on -> x: {}, y: {}, window: {:#x}", event.event_x, event.event_y, event.event);
    logger::debug("Window is {}managed", _state->manager<::Window>().is_managed(event.event) ? "" : "not ");
}

void _on_key_press(const xcb_key_press_event_t& event)
{
    xkb_keysym_t keysym = xkb_state_key_get_one_sym(_state->keyboard().state(), event.detail);
    char keysym_name[32];
    xkb_keysym_get_name(keysym, keysym_name, sizeof(keysym_name));
    logger::debug("Key pressed: {}", keysym_name);
}

void _on_key_release(const xcb_key_release_event_t& event)
{

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


