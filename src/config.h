#pragma once

// Global configs
// Does not have to be only configured on command line options

extern "C" {
#include <xcb/xproto.h>
}

// Forward declarations
class Connection; // #include "connection.h"

namespace Config
{
static const uint32_t ROOT_EVENT_MASK = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                                        XCB_EVENT_MASK_BUTTON_PRESS |
                                        XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                                        XCB_EVENT_MASK_POINTER_MOTION |
                                        XCB_EVENT_MASK_PROPERTY_CHANGE |
                                        XCB_EVENT_MASK_FOCUS_CHANGE |
                                        XCB_EVENT_MASK_ENTER_WINDOW;

extern bool replace_wm;

extern bool xkb_support;
extern bool shape_support;
extern bool randr_support;

extern uint8_t xkb_base;
extern uint8_t shape_base;
extern uint8_t randr_base;

// Disabled by default
extern bool xinerama_enabled;
// Enabled by default
extern bool randr_enabled;

// How many times should i make this function instead of making class
void init(const Connection& conn);

// Load configurations
// From .config/cube ? or idk
void load_config();
void load_extensions();

} // namespace Config
