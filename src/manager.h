#pragma once
#include "error.h"
#include <unordered_map>
#include <concepts>

// Forward declarations
class Server; // #include "server.h"

template<class Managed_t>
class Manager
{
public:
    using Managed_id        = unsigned int;
    using Managed_container = std::unordered_map<Managed_id, Managed_t*>;

    template<std::derived_from<Manager<Managed_t>> Type>
    static Type& init(Server& srv)
    {
        static Type mgr(srv);
        assert_init(mgr);
        return mgr;
    }

    virtual Managed_t* manage(const Managed_id id)   = 0;
    virtual void       unmanage(const Managed_id id) = 0;
    
    inline Managed_t* at(const Managed_id id) const
    { return _managed.at(id); }

    inline bool is_managed(const Managed_id w) const
    { return _managed.contains(w); }
    
    Manager(const Manager&)        = delete;
    Manager(Manager&&)             = delete;
    auto operator=(const Manager&) = delete;
    auto operator=(Manager&&)      = delete;

    virtual ~Manager()
    { for (const auto& pair : _managed) delete pair.second; }

protected:
    Server&           _srv;
    Managed_container _managed;

    Manager(Server& srv)
        : _srv(srv)
    {}
};
