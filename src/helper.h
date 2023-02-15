#pragma once
#include <cassert>
#include <concepts>
#include <memory>
#include <source_location>
#include <type_traits>
/**
 * Defines helper classes that can be used as mixins
 * Also functions that make that classes
 */

inline void assert_debug(const bool expr,
                         const std::string& msg = "",
                         const std::source_location loc = std::source_location::current())
{
    if (!expr) {
        std::string m = (msg.empty()) ? "" : "\nMessage: " + msg; 
        throw std::runtime_error(
            std::string("Debug failed at:\n") + 
            "\tfile    : " + loc.file_name() +
            "\tfunction: " + loc.function_name() +
            "\tline    : " + std::to_string(loc.line()) + m);
    }
}

inline void assert_runtime(const bool expr, const std::string& msg)
{
    (expr) ? void(0) : throw std::runtime_error(msg);
}

template <typename T, typename C>
concept Visitor = requires(T&& t, C&& c)
{
    t(c);
};

template <typename T, typename U>
concept derived_from = std::is_base_of<U, T>::value;

template <typename Derived>
struct Visit
{
    void accept(Visitor<Derived> auto&& visitor)
    { visitor(*static_cast<Derived*>(this)); }
    void accept(Visitor<Derived*> auto&& visitor)
    { visitor(static_cast<Derived*>(this)); }
};

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

namespace memory {

template <typename T> using c_owner = std::unique_ptr<T, decltype(free)*>;

template <typename T>
constexpr c_owner<T> c_own(T* obj)
{ return c_owner<T>(obj, free); }

}
