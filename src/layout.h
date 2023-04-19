#pragma once
#include "container.h"
#include "node.h"

class Layout : public Node<Container>
{
public:
    enum class Type
    {
        Horizontal,
        Vertical,
        Tabbed,
        Floating
    };

private:
    Type _type;

public:
    inline Type type() const noexcept
    { return _type; }

    inline void type(const Layout::Type& type) noexcept
    { _type = type; update_rect(); }

public:
    Layout(Type type) noexcept
        : _type(type)
    {}

    void update_rect() noexcept override;
    void accept(const container_visitor& visitor) noexcept override
    { visitor(*this); }

    ~Layout() noexcept
    { for (const auto& child : *this) delete &child; }
};

inline constexpr const char* layout_type_to_str(Layout::Type type)
{
    switch (type) {
    case Layout::Type::Horizontal:
        return "Horizontal";
    case Layout::Type::Vertical:
        return "Vertical";
    case Layout::Type::Tabbed:
        return "Tabbed";
    case Layout::Type::Floating:
        return "Floating";
    }
}