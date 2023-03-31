#pragma once
#include "command.h"
#include <unordered_map>
#include <xkbcommon/xkbcommon.h>

class Connection;

class Keyboard
{
protected:
    const Connection& _conn;

    xkb_context* _ctx;
    xkb_keymap*  _keymap;
    xkb_state*   _state;

    static std::unordered_map<xkb_keycode_t, Command> _bindings;

    Keyboard(const Connection& conn);

    void _clear_keymap();

public:
    constexpr xkb_state* state() const noexcept
    { return _state; }

    constexpr xkb_keymap* keymap() const noexcept
    { return _keymap; }

    virtual void update_keymap() = 0;
    virtual ~Keyboard();
};