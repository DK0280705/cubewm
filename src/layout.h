#pragma once
#include "container.h"
#include "node.h"

class Layout_frame;

class Layout : public Node<Container>
{
    void _update_rect_fn()  noexcept override;
    void _update_focus_fn() noexcept override;

public:
    enum class Containment_type
    {
        Floating,
        Horizontal,
        Vertical,
        Tabbed,
    };

private:
    Containment_type _type;

public:
    inline auto type() const noexcept -> Containment_type
    { return _type; }

    inline void type(const Containment_type& type) noexcept
    {
        _type = type;
        update_rect();
    }

public:
    explicit Layout(Containment_type type);

    ~Layout() noexcept override;
};

inline constexpr auto layout_type_to_str(Layout::Containment_type type) -> std::string_view
{
    switch (type) {
    case Layout::Containment_type::Horizontal:
        return "Horizontal";
    case Layout::Containment_type::Vertical:
        return "Vertical";
    case Layout::Containment_type::Tabbed:
        return "Tabbed";
    case Layout::Containment_type::Floating:
        return "Floating";
    }
    return "bruh";
}