#pragma once
#include "container.h"
#include "helper.h"
#include "managed.h"
#include <unordered_set>
#include <optional>

struct place;
class Window;

class Window_list
{
    std::list<Window*>          _list;
    std::unordered_set<Window*> _pos;

public:
    using Iterator       = typename decltype(_list)::iterator;
    using Const_iterator = typename decltype(_list)::const_iterator;

public:
    inline std::optref<Window> current() const noexcept
    { return _list.empty() ? std::nullopt : std::optref<Window>(*_list.back()); }

    inline bool contains(Window& window) const noexcept
    { return _pos.contains(&window); }

    DECLARE_CONTAINER_WRAPPER(_list)

public:
    void add(Window& window) noexcept;
    void focus(Const_iterator it) noexcept;
    void remove(Window& window) noexcept;
};

class Workspace : public Root<Container>
                , public Managed<unsigned int>
{
    Window_list _window_list;
    std::string _name;

public:
    Workspace(const Index id) noexcept
        : Managed(id)
        // by default the name is the id.
        , _name(std::to_string(id))
    {}

    inline Window_list& window_list() noexcept
    { return _window_list; }

    inline std::string_view name() const noexcept
    { return _name; }

public:
    void update_rect() noexcept override;

    ~Workspace() noexcept override
    { for (const auto& c : *this) delete c; }
};

