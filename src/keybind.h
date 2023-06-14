#pragma once
#include <functional>
#include <stdint.h>

enum mod_mask : uint16_t {
    shift = 1,
    lock = 2,
    control = 4,
    mod1 = 8,
    mod2 = 16,
    mod3 = 32,
    mod4 = 64,
    mod5 = 128,
    any = 32768
};

struct Keybind
{
    uint32_t keysym;
    uint16_t modifiers;
    bool operator==(const Keybind&) const = default;
};

constexpr auto hash_combine(size_t lhs, size_t rhs) -> size_t
{
    if constexpr (sizeof(size_t) >= 8) {
        lhs ^= rhs + 0x517cc1b727220a95 + (lhs << 6) + (lhs >> 2);
    } else {
        lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    }
    return lhs;
}

// Use IDE to view the value
constexpr size_t test_size1 = hash_combine(1, 2);
constexpr size_t test_size2 = hash_combine(1, 5);

template <>
struct std::hash<Keybind>
{
    auto operator()(const Keybind& k) const noexcept -> std::size_t
    {
        std::size_t h1 = std::hash<unsigned int>{}(k.keysym);
        std::size_t h2 = std::hash<unsigned int>{}(k.modifiers);
        return hash_combine(h1, h2);
    }
};