#pragma once
#include "../xkb.h"
#include "../helper.h"
#include <xcb/xcb_keysyms.h>

class Connection;
namespace X11 {
class XKB : public ::XKB
          , public Init_once<XKB>
{
    int _device_id;

public:
    XKB(const ::Connection& conn);
    void update_keymap() override final;
    virtual ~XKB();
};

xcb_keycode_t keysym_to_keycode(xcb_keysym_t keysym);

}