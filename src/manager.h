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

    using Iterator       = typename std::unordered_map<Key, Type*>::iterator;
    using Const_iterator = typename std::unordered_map<Key, Type*>::const_iterator;

private:
    Managed_container  _managed;

public:
    Manager() noexcept = default;

    Manager(const Manager&)        = delete;
    Manager(Manager&&)             = delete;
    auto operator=(const Manager&) = delete;
    auto operator=(Manager&&)      = delete;

public:
    inline Type* at(const Key& key) const
    { return _managed.at(key); }

    inline bool is_managed(const Key& key) const
    { return _managed.contains(key); }

    DECLARE_CONTAINER_WRAPPER(_managed)

public:
    template <std::derived_from<Type> Derived = Type, typename...Args>
    requires (std::constructible_from<Derived, Key, Args...>)
    Type* manage(const Key& key, Args&&... args)
    {
        assert_debug(!_managed.contains(key), "Managing already managed item");
        Type* man = new Derived(key, std::forward<Args>(args)...);
        _managed.emplace(key, man);
        this->notify(0);
        return man;
    }

    void unmanage(const Key& key)
    {
        assert_debug(_managed.contains(key), "Unmanaging unmanaged item");
        delete _managed.at(key);
        _managed.erase(key);
        this->notify(0);
    }

    void clear()
    {
        for (const auto& [_, m] : _managed)
            delete m;
        _managed.clear();
    }
};
