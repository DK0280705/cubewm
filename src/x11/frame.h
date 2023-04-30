#pragma once
#include "../frame.h"

class Layout;

namespace X11 {

class Window;
class Window_frame : public ::Window_frame
{
public:
    Window_frame(X11::Window& window);
    void focus() override {}
    void unfocus() override {}
    void update_rect() noexcept override;
};

class Layout_frame : public ::Layout_frame
{
public:
    Layout_frame(Layout& layout);
    void focus() override {}
    void unfocus() override {}
    void update_rect() noexcept override;
};

} // namespace X11