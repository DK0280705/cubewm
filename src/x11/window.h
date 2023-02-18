#pragma once
#include "../helper.h"
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <span>

class State;
class Workspace;
struct xcb_get_window_attributes_reply_t;
struct xcb_get_geometry_reply_t;

namespace X11 {
class Connection;
class Window;

namespace window {

enum class prop : uint8_t
{
    replace,
    prepend,
    append,
};

// i dunno, let the compiler guess
template <typename T>
static consteval int prop_size()
{
    if constexpr (std::is_pointer<T>())
        return sizeof(T);
    else
        return sizeof(T) * 8;
}

bool manageable(uint32_t window_id, const bool must_be_mapped = false);

void load_all(State& state);
auto load_workspace(State& state, X11::Window* window) -> ::Workspace*;

auto get_attribute(uint32_t window_id)
    -> memory::c_owner<xcb_get_window_attributes_reply_t>;

auto get_geometry(uint32_t window_id)
    -> memory::c_owner<xcb_get_geometry_reply_t>;

namespace detail {
void _cpc_impl(const window::prop mode,
               const uint32_t     wind,
               const uint8_t      prop,
               const uint8_t      type,
               const uint8_t      form,
               const uint32_t     size,
               const void*        data);

void _cp_impl(const window::prop mode,
              const uint32_t     wind,
              const uint8_t      prop,
              const uint8_t      type,
              const uint8_t      form,
              const uint32_t     size,
              const void*        data);
}

template <typename T, size_t N>
inline void change_property_c(const window::prop mode,
                              const uint32_t     wind,
                              const uint8_t      prop,
                              const uint8_t      type,
                              std::span<T, N>    data)
{
    constexpr int format = prop_size<T>(); 
    detail::_cpc_impl(mode, wind, prop, type, format, data.size(), data.data());
}

template <typename T, size_t N>
inline void change_property(const window::prop mode,
                            const uint32_t     wind,
                            const uint8_t      prop,
                            const uint8_t      type,
                            std::span<T, N>    data)
{
    constexpr int format = prop_size<T>(); 
    detail::_cp_impl(mode, wind, prop, type, format, data.size(), data.data());
}


void change_attributes(const uint32_t wind, const uint32_t mask, std::span<const uint32_t> data);
void change_attributes_c(const uint32_t wind, const uint32_t mask, std::span<const uint32_t> data);

} // namespace window
} // namespace X11
