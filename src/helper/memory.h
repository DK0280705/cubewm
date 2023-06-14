#pragma once
#include <memory>

namespace memory {

template <typename T>
concept pointer = std::is_pointer<T>::value;

template <typename T> using c_owner = std::unique_ptr<T, decltype(free)*>;
template <typename T> using owner = std::unique_ptr<T>;

template <typename T>
constexpr auto c_own(T* obj) -> c_owner<T>
{ return c_owner<T>(obj, free); }

template <typename T>
constexpr auto own(T* obj) -> owner<T>
{ return owner<T>(obj); }

template <typename T, typename...Args>
requires (std::constructible_from<T, Args...>)
constexpr auto make_owner(Args&&... args) -> owner<T>
{
    return own<T>(new T(std::forward<Args>(args)...));
}


constexpr auto validate(pointer auto && ptr)
{ if (ptr) return ptr; else throw std::logic_error("Null pointer exception"); }


template <typename Func>
requires requires(Func&& fn) { fn(); }
struct finally final
{
    Func fn;
    explicit finally(Func&& fn) noexcept
        : fn(fn)
    {}

    ~finally()
    {
        fn();
    }
};

} // namespace memory