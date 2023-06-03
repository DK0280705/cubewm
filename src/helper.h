#pragma once
#include "logger.h"
#include <cassert>
#include <concepts>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <functional>
#include <ranges>
#include <source_location>

/**
 * Defines helper classes that can be used as mixins
 * Also functions that make that classes
 */

#define DEFINE_CONTAINER_WRAPPER(container) \
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

#define DEFINE_POINTER_ITERATOR_WRAPPER(container) \
public: \
    using iterator       = pointer_iterator_wrapper<typename decltype(container)::iterator>; \
    using const_iterator = pointer_iterator_wrapper<typename decltype(container)::const_iterator>; \
    inline iterator begin() noexcept \
    { return iterator(container.begin()); } \
    inline iterator end() noexcept \
    { return iterator(container.end()); } \
    inline const_iterator begin() const noexcept \
    { return const_iterator(container.cbegin()); } \
    inline const_iterator end() const noexcept \
    { return const_iterator(container.cend()); } \
    inline const_iterator cbegin() const noexcept \
    { return const_iterator(container.cbegin()); } \
    inline const_iterator cend() const noexcept \
    { return const_iterator(container.cend()); } \

#define assert_debug(expr, msg) assert(expr && msg)

namespace std {
template <typename T>
using optref = std::optional<std::reference_wrapper<T>>;

namespace ranges {
struct __contains_fn
{
    template<std::input_iterator I, std::sentinel_for<I> S,
             class T, class Proj = std::identity>
    requires std::indirect_binary_predicate<ranges::equal_to, std::projected<I, Proj>,
                                            const T*>
    constexpr bool operator()(I first, S last, const T& value, Proj proj = {}) const
    {
        return ranges::find(std::move(first), last, value, proj) != last;
    }

    template<ranges::input_range R, class T, class Proj = std::identity>
    requires std::indirect_binary_predicate<ranges::equal_to,
                                            std::projected<ranges::iterator_t<R>, Proj>,
                                            const T*>
    constexpr bool operator()(R&& r, const T& value, Proj proj = {}) const
    {
        return (*this)(ranges::begin(r), ranges::end(r), std::move(value), proj);
    }
};

inline constexpr __contains_fn contains {};
}

}

template<std::derived_from<std::runtime_error> Err = std::runtime_error>
inline void assert_runtime(const bool expr, const std::string& msg)
{
    (expr) ? void(0) : throw Err(msg);
}

template <typename T>
concept pointer = std::is_pointer<T>::value;

template <typename T>
consteval auto func_name()
{
    const auto& loc = std::source_location::current();
    return loc.function_name();
}

template <typename T>
consteval std::string_view type_of_impl_()
{
    constexpr std::string_view f_name = func_name<T>();
    return {f_name.begin() + 37, f_name.end() - 1};
}

template <typename T>
constexpr auto type_of(T&& arg)
{
    return type_of_impl_<decltype(arg)>();
}

template <typename T>
constexpr auto type_of()
{
    return type_of_impl_<T>();
}

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
public:
    using Key                = K;
    using Observer           = std::function<void(const T&)>;
    using Observer_container = std::unordered_map<K, std::vector<Observer>>;

private:
    Observer_container _observers;

public:
    inline void connect(Key&& key, Observer observer)
    {
        _observers[std::forward<Key>(key)].emplace_back(std::move(observer));
    }

    inline void notify(Key&& key) const
    {
        const auto it = _observers.find(std::forward<Key>(key));
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

template <std::bidirectional_iterator Iterator>
requires(pointer<typename std::iterator_traits<Iterator>::value_type>)
struct pointer_iterator_wrapper final
{
private:
    Iterator _iter;

public:
    using difference_type   = ptrdiff_t;
    using iterator_category = typename std::iterator_traits<Iterator>::iterator_category;
    using value_type        = typename std::remove_pointer<typename std::iterator_traits<Iterator>::value_type>::type;
    using pointer           = typename std::iterator_traits<Iterator>::value_type;
    using reference         = typename std::remove_pointer<typename std::iterator_traits<Iterator>::value_type>::type&;

    pointer_iterator_wrapper() noexcept {};
    pointer_iterator_wrapper(const Iterator& iter) : _iter(iter) {}
    template <std::convertible_to<Iterator> It>
    pointer_iterator_wrapper(const pointer_iterator_wrapper<It>& iter) : _iter(iter.data()) {}

    operator Iterator() const noexcept
    { return _iter; }

    Iterator& data() noexcept
    { return _iter; }

    const Iterator& data() const noexcept
    { return _iter; }

    reference operator*() const noexcept
    { return *(*_iter); }
    pointer operator->() const noexcept
    { return (*_iter); }
    pointer_iterator_wrapper& operator++() noexcept
    { ++_iter; return *this; }
    pointer_iterator_wrapper operator++(int) noexcept
    { return pointer_iterator_wrapper(_iter++); }
    pointer_iterator_wrapper& operator--() noexcept
    { --_iter; return *this; }
    pointer_iterator_wrapper operator--(int) noexcept
    { return pointer_iterator_wrapper(_iter--); }
    friend bool operator==(const pointer_iterator_wrapper& rhs, const pointer_iterator_wrapper& lhs) noexcept
    { return rhs._iter == lhs._iter; }
};

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

namespace memory {

template <typename T> using c_owner = std::unique_ptr<T, decltype(free)*>;

template <typename T>
constexpr c_owner<T> c_own(T* obj)
{ return c_owner<T>(obj, free); }

constexpr auto validate(pointer auto && ptr)
{ if (ptr) return ptr; else throw std::logic_error("Null pointer exception"); }

}
