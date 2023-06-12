#pragma once
#include <optional>
#include <ranges>

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
} // namespace ranges
} // namespace std