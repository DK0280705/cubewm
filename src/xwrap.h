#pragma once
/**
 * Wrap most used xcb functions
 * So we don't have to worry about gibberish long ass function name in our code
 */

#include <cstdint>
#include <span>

// Forward declarations
typedef uint32_t xcb_window_t;
typedef uint32_t xcb_atom_t;
class Connection; // #include "connection.h"
class Window;     // #include "window.h"

namespace XWrap
{
enum CP_mode
{
    CP_REPLACE,
    CP_PREPEND,
    CP_APPEND,
};

void init(const Connection& conn);

uint32_t change_atom_property(const xcb_window_t        window,
                              const xcb_atom_t          property,
                              std::span<const uint32_t> data,
                              const CP_mode             mode = CP_REPLACE);

uint32_t change_utf8string_property(const xcb_window_t     window,
                                    const xcb_atom_t       property,
                                    std::span<const char*> data,
                                    const CP_mode          mode = CP_REPLACE);

uint32_t change_string_property(const xcb_window_t    window,
                                const xcb_atom_t      property,
                                std::span<const char> data,
                                const CP_mode         mode = CP_REPLACE);

uint32_t change_window_property(const xcb_window_t        window,
                                const xcb_atom_t          property,
                                std::span<const uint32_t> data,
                                const CP_mode             mode = CP_REPLACE);

uint32_t change_cardinal_property(const xcb_window_t        window,
                                  const xcb_atom_t          property,
                                  std::span<const uint32_t> data,
                                  const CP_mode             mode = CP_REPLACE);

uint32_t change_window_attributes(const xcb_window_t        window,
                                  const uint32_t            value_mask,
                                  std::span<const uint32_t> data);

// We have to find a way to convert event into char
void send_event(const xcb_window_t window,
                const uint32_t     event_mask,
                const char*        data);

// Don't input any dummy integer
// Check for error returned by checked functions
bool check_error(const uint32_t sequence);

void configure_window(const Window& win);
} // namespace XWrap
