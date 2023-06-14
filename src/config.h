#pragma once
#include <xcb/xproto.h>

/**
 * Configuration and constants for window manager
 */

namespace config
{

// Constants

static constexpr uint32_t GAP_SIZE = 4;

static constexpr std::string_view WM_NAME = "CubeWM";

static constexpr std::string_view WM_SN_CLASS = "cubewm-WM_Sn\0cubewm-WM_Sn";

namespace X11 {

static constexpr int CHILD_EVENT_MASK = XCB_EVENT_MASK_PROPERTY_CHANGE
                                      | XCB_EVENT_MASK_STRUCTURE_NOTIFY
                                      | XCB_EVENT_MASK_FOCUS_CHANGE;

static constexpr int FRAME_EVENT_MASK = XCB_EVENT_MASK_BUTTON_PRESS          /* …mouse is pressed/released */
                                      | XCB_EVENT_MASK_BUTTON_RELEASE
                                      | XCB_EVENT_MASK_POINTER_MOTION        /* …mouse is moved */
                                      | XCB_EVENT_MASK_EXPOSURE              /* …our window needs to be redrawn */
                                      | XCB_EVENT_MASK_STRUCTURE_NOTIFY      /* …the frame gets destroyed */
                                      | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT /* …the application tries to resize itself */
                                      | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY   /* …subwindows get notifies */
                                      | XCB_EVENT_MASK_ENTER_WINDOW;         /* …user moves cursor inside our window */

static constexpr int ROOT_EVENT_MASK = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
                                     | XCB_EVENT_MASK_BUTTON_PRESS
                                     | XCB_EVENT_MASK_STRUCTURE_NOTIFY
                                     | XCB_EVENT_MASK_POINTER_MOTION
                                     | XCB_EVENT_MASK_PROPERTY_CHANGE
                                     | XCB_EVENT_MASK_FOCUS_CHANGE
                                     | XCB_EVENT_MASK_ENTER_WINDOW;

static constexpr std::string_view FRAME_CLASS_NAME = "cube-frame";

} // namespace X11

// Runtime defined

extern bool replace_wm;

extern bool enable_xinerama;

extern bool enable_randr;

} // namespace config
