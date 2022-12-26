#pragma once
#include <fmt/compile.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

namespace Log
{
void print(const std::string& msg);

template<class Label, class Msg>
inline void log(const Label& label, const Msg& msg)
{
    print(fmt::format("{} {}", label, msg)); 
}

template<class Label, class Msg, class...Args>
constexpr void log(const Label& label, const Msg& msg, Args&&... var)
{
    print(fmt::format("{} {}", label, fmt::vformat(msg, fmt::make_format_args(var...))));
}

template<class...Args>
constexpr void info(const char* msg, Args&&... var)
{
    log("\033[1;32m[INFO]\033[0m", msg, std::forward<Args>(var)...);
}

template<class...Args>
constexpr void error(const char* msg, Args&&... var)
{
    log("\033[1;31m[ERROR]\033[0m", msg, std::forward<Args>(var)...);
}

template<class...Args>
constexpr void debug(const char* msg, Args&&... var)
{
    log("\033[1;34m[DEBUG]\033[0m", msg, std::forward<Args>(var)...);
}
}
