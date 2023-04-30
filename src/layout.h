#pragma once
#include "container.h"
#include "node.h"
#include "frame.h"

class Layout : public Node<Container>
             , public Focusable
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
    Type          _type;
    Layout_frame* _frame;

public:
    inline Type type() const noexcept
    { return _type; }

    inline void type(const Layout::Type& type) noexcept
    { _type = type; }

    inline void update_type(const Layout::Type& type) noexcept
    { _type = type; update_rect(); }

public:
    Layout(Type type);

    void focus() override
    { _frame->focus(); }

    void unfocus() override
    { _frame->unfocus(); }

    void update_rect() noexcept override;

    ~Layout() noexcept
    {
        delete _frame;
        for (const auto& child : *this) delete &child;
    }
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
    return "bruh";
}