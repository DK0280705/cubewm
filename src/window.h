#pragma once
#include "container.h"
#include "manager.h"

class Window : public Container
             , public Managed
{
protected:
    std::string _name;
    Workspace*  _ws;
    bool        _focused;

public:
    Window(Managed_id id) noexcept
        : Managed(id)
        , _ws(nullptr)
    {}

    inline std::string_view name() const
    { return _name; }

    inline Workspace* workspace() const
    { return _ws; }

    inline void workspace(Workspace* ws)
    { _ws = ws; }

    inline bool focused() const
    { return _focused; }

public:
    virtual void focus() = 0;
    virtual void unfocus() = 0;
};
