#pragma once
#include "container.h"
#include "node.h"

#include "helper/memory.h"

class Layout_frame;

class Layout : public Node<Container>
{
    void _update_rect_fn()  noexcept override;
    void _update_focus_fn() noexcept override;

public:
    enum class Type
    {
        Horizontal,
        Vertical,
        Tabbed,
        Floating
    };

private:
    Type                        _type;
    memory::owner<Layout_frame> _frame;

public:
    inline Type type() const noexcept
    { return _type; }

    inline void type(const Layout::Type& type) noexcept
    {
        _type = type;
        update_rect();
    }

public:
    explicit Layout(Type type);

    ~Layout() noexcept override;
};

inline constexpr auto layout_type_to_str(Layout::Type type) -> std::string_view
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
    return "bruh";
}