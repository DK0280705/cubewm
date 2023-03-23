#pragma once
#include <cassert>
#include <concepts>
#include <memory>
#include <stdexcept>
#include <type_traits>
/**
 * Defines helper classes that can be used as mixins
 * Also functions that make that classes
 */

#define DECLARE_ITERATOR_WRAPPER(container) \
    inline typename decltype(container)::iterator begin() noexcept \
    { return container.begin(); } \
    inline typename decltype(container)::iterator end() noexcept \
    { return container.end(); } \
    inline typename decltype(container)::const_iterator begin() const noexcept \
    { return container.begin(); } \
    inline typename decltype(container)::const_iterator end() const noexcept \
    { return container.end(); } \
    inline typename decltype(container)::const_iterator cbegin() const noexcept \
    { return container.cbegin(); } \
    inline typename decltype(container)::const_iterator cend() const noexcept \
    { return container.cend(); }


#define assert_debug(expr, msg) assert(expr && msg)

inline void assert_runtime(const bool expr, const std::string& msg)
{
    (expr) ? void(0) : throw std::runtime_error(msg);
}

template <typename T>
concept pointer = std::is_pointer<T>::value;

template <typename T, typename U>
concept derived_from = std::is_base_of<U, T>::value;

// This is not a singleton
// Just a fancy way to init something statically
template <typename Derived>
struct Init_once
{
    static Derived& init(auto&&... args)
    {
        static Derived* pinstance = nullptr;
        static Derived instance(std::forward<decltype(args)>(args)...);
        if (pinstance)
            throw std::runtime_error("init once");
        pinstance = &instance;
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

constexpr auto validate(pointer auto && ptr)
{ if (ptr) return ptr; else throw std::logic_error("Null pointer exception"); }

}
