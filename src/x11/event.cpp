#include "../logger.h"
#include "../state.h"
#include "../window_helper.h"
#include "../connection.h"
#include "constant.h"
#include "window.h"
#include "event.h"
#include "x11.h"
#include <unordered_set>
#include <xcb/xproto.h>

// xmacro(key, name);
#define SUPPORTED_EVENTS \
xmacro(MAP_REQUEST, map_request) \
xmacro(UNMAP_NOTIFY, unmap_notify) \
xmacro(DESTROY_NOTIFY, destroy_notify) \
xmacro(FOCUS_IN, focus_in) \
xmacro(FOCUS_OUT, focus_out) \

namespace X11::event {

static std::unordered_set<uint32_t> _ignored_sequences;
static State* _state;

#define xmacro(key, name) static void _on_##name (const xcb_##name##_event_t& event);
    SUPPORTED_EVENTS
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
    auto& focus_list = win->workspace()->focus_list();
    focus_list.remove(win);
    win->parent()->accept(purge(win));

    win_mgr.unmanage(event.window);
}

void _on_map_request(const xcb_map_request_event_t& event)
{
    _ignored_sequences.insert(event.sequence);
    Manager<::Window>&    win_mgr = _state->manager<::Window>();
    Manager<::Workspace>& wor_mgr = _state->manager<::Workspace>();

    if (win_mgr.is_managed(event.window)) {
        logger::debug("ignoring managed window: {}", event.window);
        return;
    }

    if (!window::manageable(event.window, false))
        return;

    ::Window* win = win_mgr.manage<X11::Window>(event.window);
    wor_mgr.current()->accept(place(win));
    xcb_map_window(X11::_conn(), win->index());

    // Set focus
    win->workspace()->focus_list().add(win);
}

void _on_focus_in(const xcb_focus_in_event_t& event)
{
    auto& wor_mgr = _state->manager<::Workspace>();
    auto& win_mgr = _state->manager<::Window>();
    auto* workspc = wor_mgr.current();
    // Update focus if root window received focus in
    if (event.event == _state->conn().xscreen()->root) {
        logger::debug("Refocusing focused window");
        if (auto* f = workspc->focus_list().current())
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
    if (win == workspc->focus_list().current()) {
        logger::debug("Ignoring current focused");
        return;
    }
    win->workspace()->focus_list().add(win);
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

} // namespace X11::event


