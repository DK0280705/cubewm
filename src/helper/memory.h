#pragma once
#include <memory>

namespace memory {

template <typename T>
concept pointer = std::is_pointer<T>::value;

template <typename T> using c_owner = std::unique_ptr<T, decltype(free)*>;

template <typename T>
constexpr c_owner<T> c_own(T* obj)
{ return c_owner<T>(obj, free); }

constexpr auto validate(pointer auto && ptr)
{ if (ptr) return ptr; else throw std::logic_error("Null pointer exception"); }


template <typename Func>
requires requires(Func&& fn) { fn(); }
struct finally final
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

} // namespace memory