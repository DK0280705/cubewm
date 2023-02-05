#pragma once
/**
 * Represents X11 atoms
 * Some of them are uninitialized global variables
 * Becareful on using them
 */

// clang-format off
#include <string>
#define SUPPORTED_ATOMS_XMACRO \
xmacro(_NET_SUPPORTED) \
xmacro(_NET_SUPPORTING_WM_CHECK) \
xmacro(_NET_WM_NAME) \
xmacro(_NET_WM_VISIBLE_NAME) \
xmacro(_NET_WM_MOVERESIZE) \
xmacro(_NET_WM_STATE_STICKY) \
xmacro(_NET_WM_STATE_FULLSCREEN) \
xmacro(_NET_WM_STATE_DEMANDS_ATTENTION) \
xmacro(_NET_WM_STATE_MODAL) \
xmacro(_NET_WM_STATE_HIDDEN) \
xmacro(_NET_WM_STATE_FOCUSED) \
xmacro(_NET_WM_STATE) \
xmacro(_NET_WM_WINDOW_TYPE) \
xmacro(_NET_WM_WINDOW_TYPE_NORMAL) \
xmacro(_NET_WM_WINDOW_TYPE_DOCK) \
xmacro(_NET_WM_WINDOW_TYPE_DIALOG) \
xmacro(_NET_WM_WINDOW_TYPE_UTILITY) \
xmacro(_NET_WM_WINDOW_TYPE_TOOLBAR) \
xmacro(_NET_WM_WINDOW_TYPE_SPLASH) \
xmacro(_NET_WM_WINDOW_TYPE_MENU) \
xmacro(_NET_WM_WINDOW_TYPE_DROPDOWN_MENU) \
xmacro(_NET_WM_WINDOW_TYPE_POPUP_MENU) \
xmacro(_NET_WM_WINDOW_TYPE_TOOLTIP) \
xmacro(_NET_WM_WINDOW_TYPE_NOTIFICATION) \
xmacro(_NET_WM_DESKTOP) \
xmacro(_NET_WM_STRUT_PARTIAL) \
xmacro(_NET_CLIENT_LIST) \
xmacro(_NET_CLIENT_LIST_STACKING) \
xmacro(_NET_CURRENT_DESKTOP) \
xmacro(_NET_NUMBER_OF_DESKTOPS) \
xmacro(_NET_DESKTOP_NAMES) \
xmacro(_NET_DESKTOP_VIEWPORT) \
xmacro(_NET_ACTIVE_WINDOW) \
xmacro(_NET_CLOSE_WINDOW) \
xmacro(_NET_MOVERESIZE_WINDOW)

#define ALL_ATOMS_XMACRO \
SUPPORTED_ATOMS_XMACRO \
xmacro(_NET_WM_USER_TIME) \
xmacro(_NET_STARTUP_ID) \
xmacro(_NET_WORKAREA) \
xmacro(_NET_REQUEST_FRAME_EXTENTS) \
xmacro(_NET_SYSTEM_TRAY_ORIENTATION) \
xmacro(_NET_SYSTEM_TRAY_VISUAL) \
xmacro(_NET_SYSTEM_TRAY_OPCODE) \
xmacro(_NET_SYSTEM_TRAY_COLORS) \
xmacro(_NET_FRAME_EXTENTS) \
xmacro(WM_STATE) \
xmacro(WM_PROTOCOLS) \
xmacro(WM_DELETE_WINDOW) \
xmacro(WM_CLIENT_LEADER) \
xmacro(WM_TAKE_FOCUS) \
xmacro(WM_WINDOW_ROLE) \
xmacro(WM_CHANGE_STATE) \
xmacro(UTF8_STRING) \
xmacro(_MOTIF_WM_HINTS) \
xmacro(_XEMBED_INFO) \
xmacro(_XEMBED) \
xmacro(MANAGER)
// clang-format on

typedef unsigned int xcb_atom_t;

namespace X11 {

namespace atom {

constexpr xcb_atom_t _NET_WM_MOVERESIZE_SIZE_TOPLEFT     = 1;
constexpr xcb_atom_t _NET_WM_MOVERESIZE_SIZE_TOP         = 1;
constexpr xcb_atom_t _NET_WM_MOVERESIZE_SIZE_TOPRIGHT    = 2;
constexpr xcb_atom_t _NET_WM_MOVERESIZE_SIZE_RIGHT       = 3;
constexpr xcb_atom_t _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT = 4;
constexpr xcb_atom_t _NET_WM_MOVERESIZE_SIZE_BOTTOM      = 5;
constexpr xcb_atom_t _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT  = 6;
constexpr xcb_atom_t _NET_WM_MOVERESIZE_SIZE_LEFT        = 7;
constexpr xcb_atom_t _NET_WM_MOVERESIZE_MOVE          = 8; /* movement only */
constexpr xcb_atom_t _NET_WM_MOVERESIZE_SIZE_KEYBOARD = 9; /* size via
                                                                keyboard */
constexpr xcb_atom_t _NET_WM_MOVERESIZE_MOVE_KEYBOARD = 10; /* move via
                                                                 keyboard */
constexpr xcb_atom_t _NET_WM_MOVERESIZE_CANCEL = 11; /* cancel operation */

constexpr xcb_atom_t _NET_MOVERESIZE_WINDOW_X      = (1 << 8);
constexpr xcb_atom_t _NET_MOVERESIZE_WINDOW_Y      = (1 << 9);
constexpr xcb_atom_t _NET_MOVERESIZE_WINDOW_WIDTH  = (1 << 10);
constexpr xcb_atom_t _NET_MOVERESIZE_WINDOW_HEIGHT = (1 << 11);

constexpr xcb_atom_t _NET_WM_DESKTOP_NONE = 0xFFFFFFF0;
constexpr xcb_atom_t _NET_WM_DESKTOP_ALL  = 0xFFFFFFFF;


#define xmacro(name) extern xcb_atom_t name;

ALL_ATOMS_XMACRO; // NOLINT
xmacro(WM_SN);    // NOLINT
#undef xmacro

xcb_atom_t  by_name(const char* name);
xcb_atom_t  by_screen(const char* base_name);
std::string name(const xcb_atom_t atom);

void init();

} // namespace atom

} // namespace X11
