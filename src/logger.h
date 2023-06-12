#pragma once
#include <fmt/compile.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace logger
{
void print_line(const std::string& msg);

inline void log(std::string_view label, const std::string& msg)
{
    print_line(fmt::format("{} {}", label, msg));
}

template <typename...T>
inline constexpr void info(fmt::format_string<T...> msg, T&&... var)
{
    log("\033[1;32m[INFO]\033[0m", fmt::format(msg, std::forward<T>(var)...));
}

template <typename...T>
inline constexpr void error(fmt::format_string<T...> msg, T&&... var)
{
    log("\033[1;31m[ERROR]\033[0m", fmt::format(msg, std::forward<T>(var)...));
}

template <typename...T>
inline constexpr void debug(fmt::format_string<T...> msg, T&&... var)
{
#ifndef NDEBUG
    log("\033[1;34m[DEBUG]\033[0m", fmt::format(msg, std::forward<T>(var)...));
#endif
}
}
