#pragma once
#include <cassert>
#include <concepts>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <functional>

/**
 * Defines helper classes that can be used as mixins
 * Also functions that make that classes
 */

#define DECLARE_CONTAINER_WRAPPER(container) \
    inline bool empty() const noexcept \
    { return container.empty(); } \
    inline typename decltype(container)::size_type size() const noexcept \
    { return container.size(); } \
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

// This is not a singleton
// Just a fancy way to init something statically
template <typename Derived>
class Init_once
{
public:
    template <typename...Args>
    requires (std::constructible_from<Derived, Args...>)
    static Derived& init(Args&&... args)
    {
        static Derived* pinstance = nullptr;
        static Derived instance(std::forward<Args>(args)...);
        if (pinstance)
            throw std::runtime_error("init once");
        pinstance = &instance;
        return instance;
    }
};

template <typename T, typename K = unsigned char>
class Observable
{
    using Observer           = std::function<void(const T&)>;
    using Observer_container = std::unordered_map<K, std::vector<Observer>>;

    Observer_container _observers;

public:
    inline void connect(K&& key, Observer observer)
    {
        _observers[std::forward<K>(key)].emplace_back(std::move(observer));
    }

    inline void notify(K&& key) const
    {
        const auto it = _observers.find(std::forward<K>(key));
        if (it != _observers.end())
            for (const auto& o : it->second)
                o(*static_cast<const T*>(this));
    }

    inline void notify_all() const
    {
        for (const auto& pair : _observers)
            for (const auto& o : pair.second)
                o(*static_cast<const T*>(this));
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
