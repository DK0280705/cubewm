#pragma once

// Global configs
// Does not have to be only configured on command line options

extern "C" {
#include <xcb/xproto.h>
}

namespace Config
{
static const uint32_t ROOT_EVENT_MASK = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;

extern bool replace_wm;

extern bool xkb_support;
extern bool shape_support;
extern bool randr_support;

extern uint8_t xkb_base;
extern uint8_t shape_base;
extern uint8_t randr_base;

} // namespace Config
