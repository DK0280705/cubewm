#pragma once
/**
 * Wrap most used xcb functions
 * So we don't have to worry about gibberish long ass function name in our code
 */

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

extern "C" {
#include <xcb/xproto.h>
}

// Forward declarations
class Connection; // #include "connection.h"
struct Window;     // #include "window.h"

namespace XWrap
{
enum class CP
{
    Replace,
    Prepend,
    Append,
};

void init(const Connection& conn);

uint32_t change_atom_property(const xcb_window_t        window,
                              const xcb_atom_t          property,
                              std::span<const uint32_t> data,
                              const CP                  mode = CP::Replace);

uint32_t change_utf8string_property(const xcb_window_t     window,
                                    const xcb_atom_t       property,
                                    std::span<const char*> data,
                                    const CP               mode = CP::Replace);

uint32_t change_string_property(const xcb_window_t    window,
                                const xcb_atom_t      property,
                                std::span<const char> data,
                                const CP              mode = CP::Replace);

uint32_t change_window_property(const xcb_window_t        window,
                                const xcb_atom_t          property,
                                std::span<const uint32_t> data,
                                const CP                  mode = CP::Replace);

uint32_t change_cardinal_property(const xcb_window_t        window,
                                  const xcb_atom_t          property,
                                  std::span<const uint32_t> data,
                                  const CP                  mode = CP::Replace);

uint32_t change_window_attributes(const xcb_window_t        window,
                                  const uint32_t            value_mask,
                                  std::span<const uint32_t> data);

auto get_window_attributes(const xcb_window_t window) -> std::unique_ptr<xcb_get_window_attributes_reply_t, void(*)(void*)>;

std::vector<xcb_window_t> get_windows();

xcb_atom_t  get_window_type(const xcb_window_t window);
std::string get_window_name(const xcb_window_t window);
std::string get_window_role(const xcb_window_t window);
uint32_t    get_window_workspace(const xcb_window_t window);
auto        get_window_class(const xcb_window_t window) -> std::pair<std::string, std::string>;

// We have to find a way to convert event into char
void send_event(const xcb_window_t window,
                const uint32_t     event_mask,
                const char*        data);

// Don't input any dummy integer
// Check for error returned by checked functions
bool check_error(const uint32_t sequence);

void configure_window(const Window& win);
} // namespace XWrap
