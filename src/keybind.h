#pragma once

typedef unsigned int xkb_keycode_t;
typedef unsigned int xkb_keysym_t;

class Keybind
{
    unsigned char _current_group;

public:
    inline void current_group(unsigned char group) noexcept
    { _current_group = group; }

    inline unsigned char current_group() const noexcept
    { return _current_group; }

public:
    virtual xkb_keycode_t keycode_from_keysym(xkb_keysym_t keysym) const = 0;
};