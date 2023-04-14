#pragma once
/**
 * A manager class
 * To manage containers.
 */
#include "managed.h"
#include "helper.h"
#include <concepts>
#include <unordered_map>

class Connection;

template <typename Type, typename Key = unsigned int>
requires (std::is_base_of<Managed<Key>, Type>::value)
class Manager : public Init_once<Manager<Type, Key>>
              , public Observable<Manager<Type, Key>>
{
public:
    using Managed_container  = std::unordered_map<Key, Type*>;

private:
    Managed_container _managed;

public:
    Manager() noexcept = default;

    Manager(const Manager&)        = delete;
    Manager(Manager&&)             = delete;
    auto operator=(const Manager&) = delete;
    auto operator=(Manager&&)      = delete;

public:
    inline Type& at(const Key& key) const
    { return *_managed.at(key); }

    inline std::optref<Type> get(const Key& key) const noexcept
    { return _managed.contains(key) ? std::optref<Type>(*_managed.at(key)) : std::nullopt; }

    inline bool contains(const Key& key) const noexcept
    { return _managed.contains(key); }

    DECLARE_CONTAINER_WRAPPER(_managed)

public:
    template <std::derived_from<Type> Derived = Type, typename...Args>
    requires (std::constructible_from<Derived, Key, Args...>)
    Derived& manage(const Key& key, Args&&... args)
    {
        assert_debug(!_managed.contains(key), "Managing already managed item");
        Derived* man = new Derived(key, std::forward<Args>(args)...);
        _managed.emplace(key, man);
        this->notify(0);
        return *man;
    }

    void unmanage(const Key& key)
    {
        assert_debug(_managed.contains(key), "Unmanaging unmanaged item");
        delete _managed.at(key);
        _managed.erase(key);
        this->notify(0);
    }

    void clear() noexcept
    {
        for (const auto& [_, m] : _managed)
            delete m;
        _managed.clear();
    }
};
