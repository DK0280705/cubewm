#include "ewmh.h"
#include "atoms.h"
#include "logger.h"
#include "server.h"

extern "C" {
#include <xcb/xcb_atom.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xproto.h>
}

#define WM_SN_CLASS "cubewm-WM_Sn\0cubewm-WM_Sn"
#define WM_SN_NAME  "cubewm WM_Sn window"

EWMH::EWMH(Server* srv) : _srv(srv)
{
#define xmacro(name)                                                                               \
    {                                                                                              \
        xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(                                    \
            _srv->conn, xcb_intern_atom(_srv->conn, 0, strlen(#name), #name), NULL);               \
        if (!reply) Log::debug("Couldn't get atom name: " #name);                                  \
        else {                                                                                     \
            name = reply->atom;                                                                    \
            free(reply);                                                                           \
        }                                                                                          \
    }
    ATOMS_XMACRO
#undef xmacro

    floating_win_atom = _srv->atom("CUBE_FLOATING_WINDOW");
    tiling_win_atom   = _srv->atom("CUBE_TILING_WINDOW");
}

EWMH* EWMH::init(Server* srv)
{
    static EWMH ewmh(srv);
    return &ewmh;
}

// Acquire last timestamp
// Never fails
bool EWMH::acquire_timestamp()
{
    xcb_generic_event_t* event;

    xcb_atom_t atom = XCB_ATOM_SUPERSCRIPT_X;
    xcb_atom_t type = XCB_ATOM_CARDINAL;

    xcb_grab_server(_srv->conn);
    xcb_change_window_attributes(_srv->conn, _srv->screen->root, XCB_CW_EVENT_MASK, (uint32_t[]) {XCB_EVENT_MASK_PROPERTY_CHANGE});
    xcb_change_property(_srv->conn, XCB_PROP_MODE_APPEND, _srv->screen->root, atom, type, 32, 0,
                        "");
    xcb_change_window_attributes(_srv->conn, _srv->screen->root, XCB_CW_EVENT_MASK, (uint32_t[]) {0});
    xcb_ungrab_server(_srv->conn);

    xcb_flush(_srv->conn);

    while ((event = xcb_wait_for_event(_srv->conn))) {
        if (XCB_EVENT_RESPONSE_TYPE(event) == XCB_PROPERTY_NOTIFY) {
            timestamp = ((xcb_property_notify_event_t*)event)->time;
            free(event);
            break;
        }
        free(event);
    }
    return true;
}

bool EWMH::acquire_selection_screen(bool replace_wm)
{
    char*      atom_name = xcb_atom_name_by_screen("WM", _srv->screen_id);
    xcb_atom_t atom      = _srv->atom(atom_name);
    wm_window_atom       = atom;
    free(atom_name);

    // Get current selection owner
    // return false if another wm is owning selection
    auto reply = xcb_get_selection_owner_reply(_srv->conn,
                                               xcb_get_selection_owner(_srv->conn, atom), 0);
    if (reply && reply->owner != XCB_NONE && !replace_wm) return false;

    // Generate main window
    wm_window = xcb_generate_id(_srv->conn);
    xcb_create_window(_srv->conn, _srv->screen->root_depth, wm_window, _srv->screen->root, -1, -1,
            1, 1, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, _srv->screen->root_visual, 0, 0);
    xcb_icccm_set_wm_class(_srv->conn, wm_window, sizeof(WM_SN_CLASS), WM_SN_CLASS);
    xcb_icccm_set_wm_name(_srv->conn, wm_window, XCB_ATOM_STRING, 8, sizeof(WM_SN_NAME) - 1,
            WM_SN_NAME);

    xcb_set_selection_owner(_srv->conn, wm_window, atom, timestamp);

    if (reply->owner != XCB_NONE) {
        xcb_get_geometry_reply_t* greply = nullptr;
        do {
            free(greply);
            greply = xcb_get_geometry_reply(_srv->conn, xcb_get_geometry(_srv->conn, reply->owner),
                                            0);
        } while (greply);
    }

    free(reply);

    // Announce that we're the new selection owner
    char                        buf[32] = {0};
    xcb_client_message_event_t* event   = (xcb_client_message_event_t*)buf;

    event->response_type  = XCB_CLIENT_MESSAGE;
    event->window         = _srv->screen->root;
    event->format         = 32;
    event->type           = MANAGER;
    event->data.data32[0] = timestamp;
    event->data.data32[1] = wm_window_atom;
    event->data.data32[2] = wm_window;

    xcb_send_event(_srv->conn, 0, _srv->screen->root, XCB_EVENT_MASK_STRUCTURE_NOTIFY,
                   (char*)event);
    return true;
}
