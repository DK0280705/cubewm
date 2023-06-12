#pragma once
#include "../xkb.h"
#include "../helper/mixins.h"
#include <xcb/xcb_keysyms.h>

class Connection;
namespace X11 {
class XKB final : public ::XKB
                , public helper::Init_once<XKB>
{
    int _device_id;

public:
    explicit XKB(::Connection& conn);
    void update_keymap() override;
    ~XKB() override;
};
} // namespace X11