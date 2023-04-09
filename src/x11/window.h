#pragma once
#include "x11.h"
#include "../window.h"
#include "../helper.h"
#include <span>
#include <cstdint>

class State;
class Workspace;
class Keyboard;
struct xcb_get_window_attributes_reply_t;
struct xcb_get_geometry_reply_t;

namespace X11 {
class Connection;

// Have a good time with this :)
class Window : public ::Window
{
    using xcb_atom_t = unsigned int;
    xcb_atom_t  _type;
    std::string _role;
    std::string _class;
    std::string _instance;
    bool        _alt_focus;

public:
    inline xcb_atom_t type() const noexcept
    { return _type; }

    inline std::string_view role() const noexcept
    { return _role; }

    inline std::string_view window_class() const noexcept
    { return _class; }

    inline std::string_view window_instance() const noexcept
    { return _instance; }

public:
    Window(Index id);

    void update_rect() override;

    void focus() override;
    void unfocus() override;
};

class Window_frame : public ::Window_frame
{
public:
    Window_frame(Window* window);
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

bool manageable(const uint32_t window_id, const bool must_be_mapped = false);

void load_all(State& state);
Workspace* load_workspace(State& state, X11::Window* window);

void grab_keys(const ::Keyboard& keyboard, const uint32_t window_id);

void grab_buttons(const uint32_t window_id);

auto get_attribute(const uint32_t window_id)
    -> memory::c_owner<xcb_get_window_attributes_reply_t>;

auto get_geometry(const uint32_t window_id)
    -> memory::c_owner<xcb_get_geometry_reply_t>;

bool has_proto(const uint32_t window_id, const uint32_t atom);

template <typename T, size_t N>
inline void change_property_c(const window::prop mode,
                              const uint32_t     wind,
                              const uint32_t     prop,
                              const uint32_t     type,
                              std::span<T, N>    data)
{
    constexpr int format = prop_size<T>();
    detail::check_error(xcb_change_property_checked(
        X11::_conn(), static_cast<uint8_t>(mode),
        wind, prop, type, format, data.size(), data.data()));
}

template <typename T, size_t N>
inline void change_property(const window::prop mode,
                            const uint32_t     wind,
                            const uint32_t     prop,
                            const uint32_t     type,
                            std::span<T, N>    data)
{
    constexpr int format = prop_size<T>();
    xcb_change_property(
        X11::_conn(), static_cast<uint8_t>(mode),
        wind, prop, type, format, data.size(), data.data());
}

template <typename T, size_t N>
inline void change_attributes(const uint32_t wind, const uint32_t mask, std::span<T, N> data)
{
    xcb_change_window_attributes(X11::_conn(), wind, mask, data.data());
}
template <typename T, size_t N>
inline void change_attributes_c(const uint32_t wind, const uint32_t mask, std::span<T, N> data)
{
    detail::check_error(xcb_change_window_attributes_checked(X11::_conn(), wind, mask, data.data()));
}

} // namespace window
} // namespace X11
