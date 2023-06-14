#include "ewmh.h"
#include "atom.h"
#include "window.h"
#include "x11.h"

#include "../config.h"
#include "../logger.h"

namespace X11::ewmh {

void update_net_supported(std::span<xcb_atom_t> atoms)
{
    window::change_property(X11::detail::root_window_id(),
                            window::prop::replace,
                            atom::_NET_SUPPORTED,
                            XCB_ATOM_ATOM,
                            atoms);
#ifndef NDEBUG
    std::vector<std::string> atom_names;
    atom_names.reserve(atoms.size());
    for (const auto a : atoms) atom_names.emplace_back(atom::name(X11::detail::conn(), a));
    logger::debug("EWMH -> updated _NET_SUPPORTING_WM_CHECK: {}", fmt::join(atom_names, ", "));
#endif
}

void update_net_supporting_wm_check(xcb_window_t window_id)
{
    window::change_property(X11::detail::root_window_id(),
                            window::prop::replace,
                            atom::_NET_SUPPORTING_WM_CHECK,
                            XCB_ATOM_WINDOW,
                            std::span{&window_id, 1});
    window::change_property(window_id,
                            window::prop::replace,
                            atom::_NET_SUPPORTING_WM_CHECK,
                            XCB_ATOM_WINDOW,
                            std::span{&window_id, 1});
    window::change_property(window_id,
                            window::prop::replace,
                            atom::_NET_WM_NAME,
                            atom::UTF8_STRING,
                            std::span{config::WM_NAME});
    logger::debug("EWMH -> updated _NET_SUPPORTING_WM_CHECK: root {:#x}, window {:#x}, name {}",
                  X11::detail::root_window_id(), window_id, config::WM_NAME);
}

void update_net_active_window(xcb_window_t window_id)
{
    // Looks like many window ids, but in fact, it's only one.
    const uint32_t window_ids[] = { window_id };
    window::change_property(X11::detail::root_window_id(),
                            window::prop::replace,
                            atom::_NET_ACTIVE_WINDOW,
                            XCB_ATOM_WINDOW,
                            std::span{window_ids});
    logger::debug("EWMH -> updated _NET_ACTIVE_WINDOW: {:#x}", window_id);
}

void update_net_current_desktop(const Workspace& workspace)
{
    const uint32_t workspace_ids[] = { workspace.index() };
    window::change_property(X11::detail::root_window_id(),
                            window::prop::replace,
                            atom::_NET_CURRENT_DESKTOP,
                            XCB_ATOM_CARDINAL,
                            std::span{workspace_ids});
    logger::debug("EWMH -> updated _NET_CURRENT_DESKTOP: {}", workspace.index());
}

void update_net_client_list(const Manager<::Window>& window_manager)
{
    std::vector<uint32_t> window_ids;
    window_ids.reserve(window_manager.size());
    for (const auto& [id, _] : window_manager)
        window_ids.emplace_back(id);

    window::change_property(X11::detail::root_window_id(),
                            window::prop::replace,
                            atom::_NET_CLIENT_LIST,
                            XCB_ATOM_WINDOW,
                            std::span{window_ids});
    logger::debug("EWMH -> updated _NET_CLIENT_LIST: {:#x}", fmt::join(window_ids, ", "));
}

void update_net_number_of_desktops(const Manager<::Workspace>& workspace_manager)
{
    const uint32_t size[] = { static_cast<uint32_t>(workspace_manager.size()) };
    window::change_property(X11::detail::root_window_id(),
                            window::prop::replace,
                            atom::_NET_NUMBER_OF_DESKTOPS,
                            XCB_ATOM_CARDINAL,
                            std::span{size});
    logger::debug("EWMH -> updated _NET_NUMBER_OF_DESKTOPS: {}", workspace_manager.size());
}

void update_net_desktop_names(const Manager<::Workspace>& workspace_manager)
{
    std::vector<std::string> names;
    names.reserve(workspace_manager.size());
    for (const auto&[_, ws] : workspace_manager)
        names.emplace_back(ws->name());

    std::vector<char> buffer;
    buffer.reserve(std::ranges::distance(std::ranges::join_view(names)) + names.size());
    for (const auto& str : names) {
        for (const auto& chr : str) buffer.emplace_back(chr);
        buffer.emplace_back('\0');
    }

    window::change_property(X11::detail::root_window_id(),
                            window::prop::replace,
                            atom::_NET_DESKTOP_NAMES,
                            atom::UTF8_STRING,
                            std::span{buffer.data(), buffer.size()});
    logger::debug("EWMH -> updated _NET_DESKTOP_NAMES: {}", fmt::join(names, ", "));
}
}