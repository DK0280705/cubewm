#pragma once
#include <type_traits>
#include <iterator>

#define HELPER_POINTER_ITERATOR_WRAPPER_DECLARE_ALIAS(container_type) \
public: \
    using iterator       = helper::pointer_iterator_wrapper<container_type::iterator>; \
    using const_iterator = helper::pointer_iterator_wrapper<container_type::const_iterator>; \

#define HELPER_POINTER_ITERATOR_WRAPPER_DECLARE_EXTENSION(container) \
public: \
    inline auto begin()        noexcept -> iterator \
    { return iterator(container.begin()); } \
    inline auto end()          noexcept -> iterator \
    { return iterator(container.end()); } \
    inline auto begin()  const noexcept -> const_iterator \
    { return const_iterator(container.cbegin()); } \
    inline auto end()    const noexcept -> const_iterator \
    { return const_iterator(container.cend()); } \
    inline auto cbegin() const noexcept -> const_iterator \
    { return const_iterator(container.cbegin()); } \
    inline auto cend()   const noexcept -> const_iterator \
    { return const_iterator(container.cend()); } \


#define HELPER_POINTER_ITERATOR_WRAPPER(container) \
public: \
    HELPER_POINTER_ITERATOR_WRAPPER_DECLARE_ALIAS(typename decltype(container)) \
    HELPER_POINTER_ITERATOR_WRAPPER_DECLARE_EXTENSION(container)

namespace helper {

template <std::bidirectional_iterator Iterator>
requires(std::is_pointer<typename std::iterator_traits<Iterator>::value_type>::value)
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

    pointer_iterator_wrapper() noexcept = default;
    pointer_iterator_wrapper(const Iterator& iter) noexcept : _iter(iter) {}
    template <std::convertible_to<Iterator> It>
    pointer_iterator_wrapper(const pointer_iterator_wrapper<It>& iter) noexcept : _iter(iter.data()) {}

    operator Iterator() const noexcept
    { return _iter; }

    auto data() noexcept -> Iterator&
    { return _iter; }
    auto data() const noexcept -> const Iterator&
    { return _iter; }

    auto operator*()     const noexcept -> reference
    { return *(*_iter); }
    auto operator->()    const noexcept -> pointer
    { return (*_iter); }
    auto operator++()    noexcept -> pointer_iterator_wrapper&
    { ++_iter; return *this; }
    auto operator++(int) noexcept -> pointer_iterator_wrapper
    { return pointer_iterator_wrapper(_iter++); }
    auto operator--()    noexcept -> pointer_iterator_wrapper&
    { --_iter; return *this; }
    auto operator--(int) noexcept -> pointer_iterator_wrapper
    { return pointer_iterator_wrapper(_iter--); }

    friend bool operator==(const pointer_iterator_wrapper& rhs, const pointer_iterator_wrapper& lhs) noexcept
    { return rhs._iter == lhs._iter; }
};

} // namespace helper