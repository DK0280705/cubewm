#pragma once
#include "../state.h"
#include <span>

/**
 * Dekstops here means workspace
 * Clients here means X11 windows
 */
using xcb_atom_t   = unsigned int;
using xcb_window_t = unsigned int;

namespace X11::ewmh {

/**
 * Update _NET_SUPPORTED
 * @param atoms
 */
void update_net_supported(std::span<xcb_atom_t> atoms);

/**
 * Update _NET_SUPPORTING_WM_CHECK
 * @param window_id The Window Manager's window id
 */
void update_net_supporting_wm_check(xcb_window_t window_id);

/**
 * Update _NET_ACTIVE_WINDOW
 * @param window_id
 */
void update_net_active_window(xcb_window_t window_id);

/**
 * Update _NET_CURRENT_DESKTOP
 * @param workspace
 */
void update_net_current_desktop(const Workspace& workspace);

/**
 * Update _NET_CLIENT_LIST
 * @param window_manager
 */
void update_net_client_list(const Manager<::Window>& window_manager);

/**
 * Update _NET_DESKTOP_NAMES
 * @param workspace_manager
 */
void update_net_desktop_names(const Manager<::Workspace>& workspace_manager);

/**
 * Update _NET_NUMBER_OF_DESKTOPS
 * @param workspace_manager
 */
void update_net_number_of_desktops(const Manager<::Workspace>& workspace_manager);

/**
 * Update _NET_SHOWING_DESKTOP
 * @param show
 */
void udpate_net_showing_desktop(bool show);

/**
 * Update _NET_WM_STATE for _NET_WM_STATE_HIDDEN
 * @param window_id
 * @param hide
 */
void update_net_wm_state_hidden(xcb_window_t window_id, bool hide);

}