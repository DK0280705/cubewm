#pragma once
#include <cassert>
#include <concepts>
#include <memory>
#include <type_traits>
/**
 * Defines helper classes that can be used as mixins
 * Also functions that make that classes
 */

#define assert_debug(expr, msg) assert(expr && msg)

inline void assert_runtime(const bool expr, const std::string& msg)
{
    (expr) ? void(0) : throw std::runtime_error(msg);
}

template <typename T, typename U>
concept derived_from = std::is_base_of<U, T>::value;

template <typename Derived>
struct Init_once
{
    static Derived& init(auto&&... args)
    {
        static Derived* _pinstance = nullptr;
        static Derived instance(std::forward<decltype(args)>(args)...);
        if (_pinstance)
            throw std::runtime_error("init once");
        _pinstance = &instance;
        return instance;
    }
};

template <typename Func>
requires requires(Func&& fn) { fn(); }
struct finally
{
    Func fn;
    finally(Func&& fn) noexcept
        : fn(fn)
    {}

    ~finally()
    {
        fn();
    }
};

namespace memory {

template <typename T> using c_owner = std::unique_ptr<T, decltype(free)*>;

template <typename T>
constexpr c_owner<T> c_own(T* obj)
{ return c_owner<T>(obj, free); }

}
