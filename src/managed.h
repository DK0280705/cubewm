#pragma once
#include <functional>

template <typename T>
concept hashable = requires (T t) {
    { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

template <hashable I>
class Managed
{
public:
    using Index = I;
private:
    Index _id;
public:
    inline explicit Managed(const Index& id) noexcept : _id(id) {}
    inline auto index() const noexcept -> Index
    { return _id; }

    virtual ~Managed() noexcept = default;
};