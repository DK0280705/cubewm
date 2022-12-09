#pragma once
#include <fmt/compile.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iostream>

namespace Log
{
void send_msg(std::ostream &o, const std::string& msg);

template <typename...>
struct Size_of
{ enum { value = 0 }; };

template <typename Ty, typename...Ty2>
struct Size_of<Ty, Ty2...>
{ enum { value = 1 }; };

template<class...Args>
constexpr void info(const char* msg, Args&&... var)
{
    if constexpr (Size_of<Args...>::value) {
        send_msg(std::cout, fmt::format("[INFO] {}", fmt::vformat(msg, fmt::make_format_args(var...))));
    } else {
        send_msg(std::cout, fmt::format("[INFO] {}", msg)); 
    }
}

template<class...Args>
constexpr void error(const char* msg, Args&&... var)
{
    if constexpr (Size_of<Args...>::value) {
        send_msg(std::cerr, fmt::format("[ERROR] {}", fmt::vformat(msg, fmt::make_format_args(var...))));
    } else {
        send_msg(std::cerr, fmt::format("[ERROR] {}", msg)); 
    }
}

template<class...Args>
constexpr void debug(const char* msg, Args&&... var)
{
    if constexpr (Size_of<Args...>::value) {
        send_msg(std::cout, fmt::format("[DEBUG] {}", fmt::vformat(msg, fmt::make_format_args(var...))));
    } else {
        send_msg(std::cout, fmt::format("[DEBUG] {}", msg)); 
    }
}
}
