#pragma once
#include "../keyboard.h"
#include "../helper.h"
#include <xcb/xcb_keysyms.h>

class Connection;
namespace X11 {
class Keyboard : public ::Keyboard
               , public Init_once<Keyboard>
{
    int _device_id;

public:
    Keyboard(const ::Connection& conn);
    void update_keymap() override final;
    virtual ~Keyboard();
};

xcb_keycode_t keysym_to_keycode(xcb_keysym_t keysym);

}