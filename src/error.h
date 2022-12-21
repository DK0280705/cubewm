#pragma once
#include <stdexcept>
#include <type_traits>

inline void assert_runtime(bool b, const char* msg)
{
    if (!b) throw std::runtime_error(msg);
}
