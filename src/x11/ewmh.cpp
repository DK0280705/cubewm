#include "ewmh.h"
#include "../state.h"
#include "../logger.h"
#include "window.h"
#include "atom.h"
#include "x11.h"
#include <algorithm>
#include <xcb/xproto.h>

namespace X11::ewmh {
void update_number_of_desktops(const State& state)
{
    logger::debug("EWMH -> updated _NET_NUMBER_OF_DESKTOPS");
    const uint32_t prop[] = { static_cast<uint32_t>(state.manager<::Workspace>().size()) };
    window::change_property(X11::detail::root_window_id(),
                            window::prop::replace,
                            atom::_NET_NUMBER_OF_DESKTOPS,
                            XCB_ATOM_CARDINAL,
                            std::span{prop});
}

void update_current_desktop(const State& state)
{
    logger::debug("EWMH -> updated _NET_CURRENT_DESKTOP");
    const uint32_t prop[] = { state.current_workspace().index() };
    window::change_property(X11::detail::root_window_id(),
                            window::prop::replace,
                            atom::_NET_CURRENT_DESKTOP,
                            XCB_ATOM_CARDINAL,
                            std::span{prop});
}

void update_desktop_names(const State& state)
{
    logger::debug("EWMH -> updated _NET_DESKTOP_NAMES");
    std::string names;
    std::size_t names_length = 0;
    for (const auto&[id, ws] : state.manager<::Workspace>()) {
        names += std::string(ws->name()) + '\0';
        names_length += ws->name().size() + 1;
    }
    window::change_property(X11::detail::root_window_id(),
                            window::prop::replace,
                            atom::_NET_DESKTOP_NAMES,
                            atom::UTF8_STRING,
                            std::span{names.data(), names_length});
}

void update_active_window(uint32_t window)
{
    logger::debug("EWMH -> updated _NET_ACTIVE_WINDOW");
    const uint32_t prop[] = { window };
    window::change_property(X11::detail::root_window_id(),
                            window::prop::replace,
                            atom::_NET_ACTIVE_WINDOW,
                            XCB_ATOM_WINDOW,
                            std::span{prop});
}

void update_client_list(const State& state)
{
    logger::debug("EWMH -> updated _NET_CLIENT_LIST");
    const auto& mgr = state.manager<::Window>();
    std::vector<uint32_t> prop(mgr.size());
    uint32_t i = 0;
    for (const auto& [id, win] : mgr) {
        prop[i] = id;
        ++i;
    }

    window::change_property(X11::detail::root_window_id(),
                            window::prop::replace,
                            atom::_NET_CLIENT_LIST,
                            XCB_ATOM_WINDOW,
                            std::span{prop});
}
}