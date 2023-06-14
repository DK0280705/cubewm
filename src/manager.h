#pragma once
/**
 * A manager class
 * To manage containers.
 */
#include "managed.h"
#include "helper/mixins.h"
#include "helper/std_extension.h"
#include <concepts>
#include <unordered_map>

class Connection;

template <typename Type>
requires (std::is_base_of<Managed<typename Type::Index>, Type>::value)
class Manager : public helper::Init_once<Manager<Type>>
              , public helper::Observable<Manager<Type>>
{
public:
    using Key                = typename Type::Index;
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
    inline auto at(const Key& key) const -> Type&
    { return *_managed.at(key); }

    inline auto operator[](const Key& key) const noexcept -> std::optref<Type>
    { return _managed.contains(key) ? std::optref<Type>(*_managed.at(key)) : std::nullopt; }

    inline bool contains(const Key& key) const noexcept
    { return _managed.contains(key); }

    HELPER_CONTAINER_WRAPPER(_managed)

public:
    template <std::derived_from<Type> Derived = Type, typename...Args>
    requires (std::constructible_from<Derived, Key, Args...>)
    auto manage(const Key& key, Args&&... args) -> Derived&
    {
        assert_runtime<Existence_error>(!_managed.contains(key), "Managing already managed key");
        const auto& [it, _] = _managed.emplace(key, new Derived(key, std::forward<Args>(args)...));
        assert(_);
        this->notify_all();
        return *static_cast<Derived*>(it->second);
    }

    void unmanage(const Key& key)
    {
        assert_runtime<Existence_error>(_managed.contains(key), "Unmanaging unmanaged item");
        delete _managed.at(key);
        _managed.erase(key);
        this->notify_all();
    }

    void clear() noexcept
    {
        for (const auto& [_, m] : _managed)
            delete m;
        _managed.clear();
        this->notify_all();
    }
};
