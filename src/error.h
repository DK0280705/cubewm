#pragma once
#include <stdexcept>
#include <cassert>

#define assert_debug(expr, msg) assert(expr && msg)

class Cube_exception : public std::runtime_error
{
public:
    explicit Cube_exception(const std::string& message) noexcept
        : std::runtime_error(message)
    {}
};

// wtf is this name
class Existence_error : public Cube_exception
{
public:
    explicit Existence_error(const std::string& message) noexcept
        : Cube_exception(message)
    {}
};

class Connection_error : public Cube_exception
{
public:
    explicit Connection_error(const std::string& message) noexcept
        : Cube_exception(message)
    {}
};

class Display_error : public Cube_exception
{
public:
    explicit Display_error(const std::string& message) noexcept
        : Cube_exception(message)
    {}
};

class Server_error : public Cube_exception
{
public:
    explicit Server_error(const std::string& message) noexcept
        : Cube_exception(message)
    {}
};

template<std::derived_from<std::runtime_error> Err = std::runtime_error>
inline void assert_runtime(const bool expr, const std::string& msg)
{
    (expr) ? void(0) : throw Err(msg);
}