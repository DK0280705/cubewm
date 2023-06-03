#pragma once
#include "keybind.h"
#include <unordered_map>
#include <xkbcommon/xkbcommon.h>

class Connection;

class XKB
{
protected:
    const Connection& _conn;
    xkb_context*      _ctx;
    xkb_keymap*       _keymap;
    xkb_state*        _state;

    XKB(const Connection& conn);
    void _clear_keymap();

public:
    inline auto context() const noexcept -> xkb_context*
    { return _ctx;    }
    inline auto keymap()  const noexcept -> xkb_keymap*
    { return _keymap; }
    inline auto state()   const noexcept -> xkb_state*
    { return _state;  }

    virtual void update_keymap() = 0;
    virtual ~XKB();
};

auto create_keybind(XKB& xkb, uint32_t keycode, uint16_t modifiers) noexcept -> Keybind;