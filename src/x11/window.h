#pragma once
#include "x11.h"
#include "../window.h"
#include "../helper/memory.h"

#include <span>
#include <vector>
#include <xcb/xcb_icccm.h>

// Forward declarations
class State;
class Workspace;

struct X11_window_property
{
    std::string           name;
    uint32_t              type{};
    std::string           role;
    xcb_icccm_wm_hints_t  wm_hints{};
    std::vector<uint32_t> protocols;
    struct WM_class
    {
        std::string wclass;
        std::string instance;
    } wm_class;
};

namespace X11 {

namespace detail {
// i dunno, let the compiler guess
// Not very important, keep it one line
template <typename T>
static consteval auto prop_size() -> int
{ if constexpr (std::is_pointer<T>()) return sizeof(T); else return sizeof(T) * 8; }
} // namespace detail

class Connection;

class Window_impl final : public ::Window::Impl
{
    // Don't modify window inside implementation.
    const Window&       _window;
    X11_window_property _xprop;
    bool                _do_not_focus;

public:
    explicit Window_impl(const Window& window);

    void update_rect()                      noexcept override;
    void update_focus()                     noexcept override;
    void update_state(Window::State wstate) noexcept override;

    void kill() noexcept override;

    ~Window_impl() noexcept override;
};

namespace window {

enum class prop : uint8_t
{
    replace,
    prepend,
    append,
};

/**
 * @brief Get X11 window attributes
 * @param window_id
 * @return memory::c_owner<xcb_get_window_attributes_reply_t>
 */
auto get_attribute(uint32_t window_id) noexcept
    -> memory::c_owner<xcb_get_window_attributes_reply_t>;

/**
 * @brief Get X11 window geometry (size, etc).
 * @param window_id
 * @return memory::c_owner<xcb_get_geometry_reply_t>
 */
auto get_geometry(uint32_t window_id) noexcept
    -> memory::c_owner<xcb_get_geometry_reply_t>;

/**
 * @brief Configure X11 window rect
 * @param window_id
 * @param rect
 */
void configure_rect(uint32_t window_id, const Vector2D& rect) noexcept;

/**
 * @brief Grab all keys for an X11 window
 * @param window_id
 * @param state
 */
void grab_keys(uint32_t window_id, const State& state) noexcept;

/**
 * @brief Grab all buttons for an X11 window.
 * You might want to use it only for root window
 * @param window_id
 */
void grab_buttons(uint32_t window_id) noexcept;

/**
 * @brief Load all X11 Windows into State object.
 * @param state
 */
void load_all(State& state);

/**
 * @brief Manages a window and load it into State object.
 * @param window_id
 * @param state
 */
void manage(uint32_t window_id, State& state);

/**
 * @brief Send WM_TAKE_FOCUS protocol to a window
 * @param window_id
 */
void send_take_focus(uint32_t window_id) noexcept;

/**
 * @brief Set input focus to window
 * @param window_id
 */
void set_input_focus(uint32_t window_id) noexcept;

/**
 * @brief Change window property
 * @tparam T Data type
 * @tparam N Data size
 * @param wind window id
 * @param mode window::prop
 * @param prop property to change
 * @param type property type
 * @param data
 */
template <typename T, size_t N>
inline void change_property(const uint32_t     wind,
                            const window::prop mode,
                            const uint32_t     prop,
                            const uint32_t     type,
                            std::span<T, N>    data) noexcept
{
    constexpr int format = detail::prop_size<T>();
    xcb_change_property(
        X11::detail::conn(), static_cast<uint8_t>(mode),
        wind, prop, type, format, data.size(), data.data());
}

/**
 * @brief Change window property, throws if fail
 * @tparam T Data type
 * @tparam N Data size
 * @param wind window id
 * @param mode window::prop
 * @param prop property to change
 * @param type property type
 * @param data
 */
template <typename T, size_t N>
inline void change_property_c(const uint32_t     wind,
                              const window::prop mode,
                              const uint32_t     prop,
                              const uint32_t     type,
                              std::span<T, N>    data)
{
    constexpr int format = detail::prop_size<T>();
    X11::detail::check_error(xcb_change_property_checked(
        X11::detail::conn(), static_cast<uint8_t>(mode),
        wind, prop, type, format, data.size(), data.data()));
}

/**
 * @brief Change window attributes
 * @tparam T Data type
 * @tparam N Data size
 * @param window_id
 * @param mask attribute mask
 * @param data
 */
template <typename T, size_t N>
inline void change_attributes(const uint32_t window_id, const uint32_t mask, std::span<T, N> data) noexcept
{
    xcb_change_window_attributes(X11::detail::conn(), window_id, mask, data.data());
}

/**
 * @brief Change window attributes, throws if fail
 * @tparam T Data type
 * @tparam N Data size
 * @param window_id
 * @param mask attribute mask
 * @param data
 */
template <typename T, size_t N>
inline void change_attributes_c(const uint32_t window_id, const uint32_t mask, std::span<T, N> data)
{
    X11::detail::check_error(xcb_change_window_attributes_checked(X11::detail::conn(), window_id, mask, data.data()));
}

} // namespace window
} // namespace X11
