#include "ewmh.h"
#include "../state.h"
#include "window.h"
#include "atom.h"
#include "x11.h"
#include <algorithm>
#include <xcb/xproto.h>

namespace X11::ewmh {
void update_number_of_desktops(const Manager<::Workspace>& mgr)
{
    const uint32_t prop[] = { static_cast<uint32_t>(mgr.size()) };
    window::change_property(window::prop::replace, X11::_root_window_id(),
                            atom::_NET_NUMBER_OF_DESKTOPS, XCB_ATOM_CARDINAL,
                            std::span{prop});
}

void update_current_desktop(const State& state)
{
    const uint32_t prop[] = { state.current_workspace()->index() };
    window::change_property(window::prop::replace, X11::_root_window_id(),
                            atom::_NET_CURRENT_DESKTOP, XCB_ATOM_CARDINAL,
                            std::span{prop});
}

void update_desktop_names(const Manager<::Workspace>& mgr)
{
    std::string names;
    std::size_t names_length = 0;
    for (const auto&[id, ws] : mgr) {
        names += std::string(ws->name()) + "\0";
        names_length += ws->name().size() + 1;
    }
    window::change_property(window::prop::replace, X11::_root_window_id(),
                            atom::_NET_DESKTOP_NAMES, atom::UTF8_STRING,
                            std::span{names.data(), names_length});
}

void update_active_window(uint32_t window)
{
    const uint32_t prop[] = { window };
    window::change_property(window::prop::replace, X11::_root_window_id(),
                            atom::_NET_ACTIVE_WINDOW, XCB_ATOM_WINDOW,
                            std::span{prop});
}

void update_client_list(const Manager<::Window>& mgr)
{
    std::vector<uint32_t> prop(mgr.size());
    uint32_t i = 0;
    for (const auto& [id, win] : mgr) {
        prop[i] = id;
        ++i;
    }

    window::change_property(window::prop::replace, X11::_root_window_id(),
                            atom::_NET_CLIENT_LIST, XCB_ATOM_WINDOW,
                            std::span{prop});
}
}