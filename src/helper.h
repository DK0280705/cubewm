#pragma once
#include <bits/iterator_concepts.h>
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
}

template<std::derived_from<std::runtime_error> Err = std::runtime_error>
inline void assert_runtime(const bool expr, const std::string& msg)
{
    (expr) ? void(0) : throw Err(msg);
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
public:
    using Observer           = std::function<void(const T&)>;
    using Observer_container = std::unordered_map<K, std::vector<Observer>>;

private:
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

namespace memory {

template <typename T> using c_owner = std::unique_ptr<T, decltype(free)*>;

template <typename T>
constexpr c_owner<T> c_own(T* obj)
{ return c_owner<T>(obj, free); }

constexpr auto validate(pointer auto && ptr)
{ if (ptr) return ptr; else throw std::logic_error("Null pointer exception"); }

}
