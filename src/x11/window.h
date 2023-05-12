#pragma once
#include "x11.h"
#include "../window.h"
#include "../helper.h"
#include <span>

// Forward declarations
class State;
class Workspace;
class Keyboard;
class Mouse;

namespace X11 {
class Connection;

// Have a good time with this :)
class Window : public ::Window
{
    xcb_atom_t  _type;
    std::string _role;
    std::string _class;
    std::string _instance;
    bool        _alt_focus;

public:
    Window(Index id);

    void update_rect() noexcept override;

    void focus() override;
    void unfocus() override;
};

namespace window {

enum class prop : uint8_t
{
    replace,
    prepend,
    append,
};

// i dunno, let the compiler guess
// Not very important, keep it one line
template <typename T>
static consteval int prop_size()
{ if constexpr (std::is_pointer<T>()) return sizeof(T); else return sizeof(T) * 8; }

auto get_attribute(const uint32_t window_id) noexcept
    -> memory::c_owner<xcb_get_window_attributes_reply_t>;

auto get_geometry(const uint32_t window_id) noexcept
    -> memory::c_owner<xcb_get_geometry_reply_t>;

bool has_proto(const uint32_t window_id, const uint32_t atom) noexcept;


void configure_rect(const uint32_t window_id, const Vector2D& rect) noexcept;

void grab_keys(const uint32_t window_id, const ::Keyboard& keyboard) noexcept;

void grab_buttons(const uint32_t window_id) noexcept;

void load_all(State& state);

void manage(const uint32_t window_id, State& state, const bool is_starting_up);
void unmanage(const uint32_t window_id, State& state);

void send_take_focus(const uint32_t window_id) noexcept;

void set_input_focus(const uint32_t window_id) noexcept;


template <typename T, size_t N>
inline void change_property(const uint32_t     wind,
                            const window::prop mode,
                            const uint32_t     prop,
                            const uint32_t     type,
                            std::span<T, N>    data) noexcept
{
    constexpr int format = prop_size<T>();
    xcb_change_property(
        X11::detail::conn(), static_cast<uint8_t>(mode),
        wind, prop, type, format, data.size(), data.data());
}

template <typename T, size_t N>
inline void change_property_c(const uint32_t     wind,
                              const window::prop mode,
                              const uint32_t     prop,
                              const uint32_t     type,
                              std::span<T, N>    data)
{
    constexpr int format = prop_size<T>();
    detail::check_error(xcb_change_property_checked(
        X11::detail::conn(), static_cast<uint8_t>(mode),
        wind, prop, type, format, data.size(), data.data()));
}

template <typename T, size_t N>
inline void change_attributes(const uint32_t window_id, const uint32_t mask, std::span<T, N> data) noexcept
{
    xcb_change_window_attributes(X11::detail::conn(), window_id, mask, data.data());
}

template <typename T, size_t N>
inline void change_attributes_c(const uint32_t window_id, const uint32_t mask, std::span<T, N> data)
{
    detail::check_error(xcb_change_window_attributes_checked(X11::detail::conn(), window_id, mask, data.data()));
}

} // namespace window
} // namespace X11
