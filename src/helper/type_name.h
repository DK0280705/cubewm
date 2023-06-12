#pragma once
#include <string_view>
#include <array>

namespace helper {

namespace detail {

template <std::size_t...Idxs>
consteval auto substr_as_arr(std::string_view str, std::index_sequence<Idxs...>)
{
    return std::array{str[Idxs]..., '\n'};
}

template <typename T>
consteval auto type_name_arr()
{
    #if defined(__clang__)
    constexpr std::string_view prefix = "[T = ";
    constexpr std::string_view suffix = "]";
    #elif defined(__GNUC__)
    constexpr std::string_view prefix = "[with T = ";
    constexpr std::string_view suffix = "]";
    #else
    #error Unsupported compiler
    #endif
    constexpr std::string_view func_name = __PRETTY_FUNCTION__;

    constexpr auto start = func_name.find(prefix) + prefix.size();
    constexpr auto end   = func_name.rfind(suffix);
    static_assert(start < end);

    constexpr auto name  = func_name.substr(start, (end - start));
    return substr_as_arr(name, std::make_index_sequence<name.size()>());
}

template <typename T>
struct type_name_holder
{
    static inline constexpr auto value = type_name_arr<T>();
};
} // namespace detail

template <typename T>
consteval auto type_name() -> std::string_view
{
    constexpr auto& value = detail::type_name_holder<T>::value;
    return std::string_view{value.data(), value.size()};
}

} // namespace helper