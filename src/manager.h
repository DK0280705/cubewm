#pragma once
/**
 * A manager class
 * To manage containers.
 */
#include "managed.h"
#include "helper.h"
#include "visitor.h"
#include <unordered_map>

class Connection;

template <derived_from<Managed> Managed_t>
class Manager : public Init_once<Manager<Managed_t>>
              , public Visitable_static<Manager<Managed_t>>
{
public:
    using Managed_id        = unsigned int;
    using Managed_container = std::unordered_map<Managed_id, Managed_t*>;

    using Iterator       = typename std::unordered_map<Managed_id, Managed_t*>::iterator;
    using Const_iterator = typename std::unordered_map<Managed_id, Managed_t*>::const_iterator;

private:
    Managed_container _managed;

public:   
    Manager() noexcept = default;

    Manager(const Manager&)        = delete;
    Manager(Manager&&)             = delete;
    auto operator=(const Manager&) = delete;
    auto operator=(Manager&&)      = delete;

public:
    inline bool empty() const noexcept
    { return _managed.empty(); }

    inline std::size_t size() const noexcept
    { return _managed.size(); }

    inline Managed_t* at(const Managed_id id) const
    { return _managed.at(id); }

    inline bool is_managed(const Managed_id w) const
    { return _managed.contains(w); }

    DECLARE_ITERATOR_WRAPPER(_managed)

public:
    template <derived_from<Managed_t> Derived_t = Managed_t, typename...Args>
    Managed_t* manage(const Managed_id id, Args&&... args)
    {
        Managed_t* man = new Derived_t(id, std::forward<Args>(args)...);
        _managed.emplace(id, man);
        return man;
    }

    void unmanage(const Managed_id id)
    {
        delete _managed.at(id);
        _managed.erase(id);
    }
};
