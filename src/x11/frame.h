#pragma once
#include "../frame.h"

class Layout;

namespace X11 {

class Window;
class Window_frame : public ::Window_frame
{
    void _update_rect_fn()  noexcept override;
    void _update_focus_fn() noexcept override;

public:
    explicit Window_frame(X11::Window& window);

    ~Window_frame() noexcept override;
};

class Layout_frame : public ::Layout_frame
{
    void _update_rect_fn()  noexcept override;
    void _update_focus_fn() noexcept override;

public:
    explicit Layout_frame(Layout& layout);

    ~Layout_frame() noexcept override;
};

} // namespace X11