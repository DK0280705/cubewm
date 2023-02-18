#include "../logger.h"
#include "../state.h"
#include "../window_helper.h"
#include "../connection.h"
#include "window.h"
#include "event_handler.h"
#include "x11.h"
#include <unordered_set>
#include <xcb/xproto.h>

// xmacro(key, name);
#define SUPPORTED_EVENTS \
xmacro(MAP_REQUEST, map_request) \
xmacro(UNMAP_NOTIFY, unmap_notify) \
xmacro(DESTROY_NOTIFY, destroy_notify) \


namespace X11 {

class Event_handler_impl : public Init_once<Event_handler_impl>
{
    State&                       _state;
    std::unordered_set<uint32_t> _ignored_sequences;

public:
    Event_handler_impl(State& state)
        : _state(state)
    {}

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

private:
#define xmacro(key, name) void _on_##name (const xcb_##name##_event_t& event);
    SUPPORTED_EVENTS
#undef xmacro
};

Event_handler::Event_handler(State& state)
    : _impl(Event_handler_impl::init(state))
{
}

void Event_handler::handle(const Event& event)
{
    _impl.handle(event);
}


// Implementations for each events

void Event_handler_impl::_on_destroy_notify(const xcb_destroy_notify_event_t& event)
{
    const xcb_unmap_notify_event_t unmap {
        .sequence = event.sequence,
        .event    = event.event,
        .window   = event.window
    };
    _on_unmap_notify(unmap);
}

void Event_handler_impl::_on_unmap_notify(const xcb_unmap_notify_event_t& event)
{
    Manager<::Window>& win_mgr = _state.manager<::Window>();
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

void Event_handler_impl::_on_map_request(const xcb_map_request_event_t& event)
{
    _ignored_sequences.insert(event.sequence);
    Manager<::Window>&    win_mgr = _state.manager<::Window>();
    Manager<::Workspace>& wor_mgr = _state.manager<::Workspace>();

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

} // namespace X11


