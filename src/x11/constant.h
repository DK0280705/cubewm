#include <xcb/xproto.h>

namespace X11 {
namespace constant {

static const uint32_t CHILD_EVENT_MASK = XCB_EVENT_MASK_PROPERTY_CHANGE |
                                         XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                                         XCB_EVENT_MASK_FOCUS_CHANGE;

static const uint32_t ROOT_EVENT_MASK = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                                        XCB_EVENT_MASK_BUTTON_PRESS |
                                        XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                                        XCB_EVENT_MASK_POINTER_MOTION |
                                        XCB_EVENT_MASK_PROPERTY_CHANGE |
                                        XCB_EVENT_MASK_FOCUS_CHANGE |
                                        XCB_EVENT_MASK_ENTER_WINDOW;
}
}
