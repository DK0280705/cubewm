#pragma once
#include <concepts>
#include <memory>
#include <cassert>
/**
 * Defines helper classes that can be used as mixins
 * Also functions that make that classes
 */

#define assert_debug(exp, msg) assert(((void)msg, exp))

constexpr void assert_runtime(const bool expr, const std::string& msg)
{
    (expr) ? void(0) : throw std::runtime_error(msg);
}

template <typename T, typename C>
concept visitor = requires(T&& t, C&& c)
{
    { t(c) };
};

template <typename T, typename U>
concept derived_from = std::is_base_of<U, T>::value;

template <typename Derived>
class Visit
{
public:
    template<visitor<Derived> V>
    void accept(V&& visitor)
    {
        visitor(*static_cast<Derived*>(this));
    }
};

template <typename Derived>
class Init_once
{
public:
    template<typename...Args>
    static Derived& init(Args&&... args)
    {
        static Derived* _pinstance = nullptr;
        static Derived instance(std::forward<Args>(args)...);
        if (_pinstance)
            throw std::runtime_error("init once");
        _pinstance = &instance;
        return instance;
    }
};

template <typename T>
class Span_dynamic
{
    T*     _data;
    size_t _size;

public:
    inline Span_dynamic(T* data, size_t size) noexcept
        : _data(data)
        , _size(size)
    {}

    inline T* begin()
    { return _data; }

    inline T* end()
    { return _data + _size; }

    inline T* cbegin() const
    { return _data; }

    inline T* cend() const
    { return _data + _size; }
};

namespace memory {

template <typename T> using c_owner = std::unique_ptr<T, decltype(free)*>;

template <typename T>
constexpr c_owner<T> c_own(T* obj)
{ return c_owner<T>(obj, free); }

}
