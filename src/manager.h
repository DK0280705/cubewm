#pragma once
/**
 * A manager class
 * To manage containers.
 */

#include "helper.h"
#include "visitor.h"
#include <unordered_map>

class Connection;

class Managed
{
public:
    using Managed_id = unsigned int;

private:
    Managed_id _id;

public:
    Managed(const Managed_id id) noexcept
        : _id(id)
    {}

    inline Managed_id index() const noexcept
    { return _id; }
};

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
    Managed_id        _current;
    Managed_container _managed;

public:   
    Manager()
        : _current(0)
    {}

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

    inline Managed_t* current() const
    { return _managed.at(_current); }

    inline void set_current(uint16_t index)
    {
        assert_debug(is_managed(index), "index is not managed");
        _current = index;
    }

    inline bool is_managed(const Managed_id w) const
    { return _managed.contains(w); }

public: 
    inline Iterator begin() noexcept
    { return _managed.begin(); }

    inline Iterator end() noexcept
    { return _managed.end(); }

    inline Const_iterator cbegin() noexcept
    { return _managed.cbegin(); }

    inline Const_iterator cend() noexcept
    { return _managed.cend(); }

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
