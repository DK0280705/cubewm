#pragma once
#include <string_view>
#include <xcb/xproto.h>

namespace X11 {
namespace constant {

static constexpr uint32_t CHILD_EVENT_MASK = XCB_EVENT_MASK_PROPERTY_CHANGE
                                           | XCB_EVENT_MASK_STRUCTURE_NOTIFY
                                           | XCB_EVENT_MASK_FOCUS_CHANGE;

static constexpr uint32_t FRAME_EVENT_MASK = XCB_EVENT_MASK_BUTTON_PRESS          /* …mouse is pressed/released */
                                           | XCB_EVENT_MASK_BUTTON_RELEASE
                                           | XCB_EVENT_MASK_POINTER_MOTION        /* …mouse is moved */
                                           | XCB_EVENT_MASK_EXPOSURE              /* …our window needs to be redrawn */
                                           | XCB_EVENT_MASK_STRUCTURE_NOTIFY      /* …the frame gets destroyed */
                                           | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT /* …the application tries to resize itself */
                                           | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY   /* …subwindows get notifies */
                                           | XCB_EVENT_MASK_ENTER_WINDOW;         /* …user moves cursor inside our window */
 

static constexpr uint32_t ROOT_EVENT_MASK = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
                                          | XCB_EVENT_MASK_BUTTON_PRESS
                                          | XCB_EVENT_MASK_STRUCTURE_NOTIFY
                                          | XCB_EVENT_MASK_POINTER_MOTION
                                          | XCB_EVENT_MASK_PROPERTY_CHANGE
                                          | XCB_EVENT_MASK_FOCUS_CHANGE
                                          | XCB_EVENT_MASK_ENTER_WINDOW;

static constexpr std::string_view FRAME_CLASS_NAME = "cube-frame";

}
}
