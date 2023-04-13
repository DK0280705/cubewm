#pragma once
#include "binding.h"
#include "manager.h"
#include "keybind.h"
#include <unordered_map>
#include <xkbcommon/xkbcommon.h>

class Connection;

class Keyboard
{

protected:
    const Connection& _conn;
    xkb_context*      _ctx;
    xkb_keymap*       _keymap;
    xkb_state*        _state;

    Manager<Binding, Keybind> _binding_manager;

    Keyboard(const Connection& conn);
    void _clear_keymap();

public:
    inline xkb_state* state() const noexcept
    { return _state; }

    constexpr const Manager<Binding, Keybind>& bindings() const noexcept
    { return _binding_manager; }

    virtual void update_keymap() = 0;
    virtual ~Keyboard();
};