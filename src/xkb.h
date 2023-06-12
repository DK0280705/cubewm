#pragma once
#include "keybind.h"
#include <unordered_map>
#include <xkbcommon/xkbcommon.h>

class Connection;

class XKB
{
protected:
    Connection&  _conn;
    xkb_context* _ctx;
    xkb_keymap*  _keymap;
    xkb_state*   _state;

    static inline XKB* _instance = nullptr;
    explicit XKB(Connection& conn);
    void _clear_keymap();

public:
    static auto instance() noexcept -> XKB&;

    inline auto context() const noexcept -> xkb_context*
    { return _ctx;    }
    inline auto keymap()  const noexcept -> xkb_keymap*
    { return _keymap; }
    inline auto state()   const noexcept -> xkb_state*
    { return _state;  }

    /**
     * @brief Creates a Keybind with specified keycode and modifiers
     * @param keycode
     * @param modifiers
     * @return
     */
    static auto create_keybind(xkb_keycode_t keycode, xkb_mod_mask_t modifiers) noexcept -> Keybind;

    virtual void update_keymap() = 0;
    virtual ~XKB();
};