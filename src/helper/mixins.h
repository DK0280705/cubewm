#pragma once
#include "../error.h"
#include <concepts>
#include <functional>

#define HELPER_CONTAINER_WRAPPER(container) \
    inline bool empty()  const noexcept \
    { return container.empty(); } \
    inline auto size()   const noexcept -> typename decltype(container)::size_type \
    { return container.size(); } \
    inline auto begin()        noexcept -> typename decltype(container)::iterator \
    { return container.begin(); } \
    inline auto end()          noexcept -> typename decltype(container)::iterator \
    { return container.end(); } \
    inline auto begin()  const noexcept -> typename decltype(container)::const_iterator \
    { return container.begin(); } \
    inline auto end()    const noexcept -> typename decltype(container)::const_iterator \
    { return container.end(); } \
    inline auto cbegin() const noexcept -> typename decltype(container)::const_iterator\
    { return container.cbegin(); } \
    inline auto cend()   const noexcept -> typename decltype(container)::const_iterator\
    { return container.cend(); }

namespace helper {

// This is not a singleton
// Just a fancy way to init something statically
template <typename Derived>
class Init_once
{
public:
    template <typename...Args>
    requires (std::constructible_from<Derived, Args...>)
    static auto init(Args&&... args) -> Derived&
    {
        static Derived* pinstance = nullptr;
        assert_runtime<Existence_error>(!pinstance, "init once");

        static Derived instance(std::forward<Args>(args)...);
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

} // namespace helper