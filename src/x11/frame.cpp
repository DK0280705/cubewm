#include "frame.h"
#include "constant.h"
#include "x11.h"
#include "window.h"
#include "../connection.h"
#include "../logger.h"

namespace X11 {

Window_frame::Window_frame(Window& window)
    : ::Window_frame(xcb_generate_id(X11::detail::conn()), window)
{
    const uint32_t mask = XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK ;
    const uint32_t values[] = { 1, constant::FRAME_EVENT_MASK };
    xcb_create_window(X11::detail::conn(), XCB_COPY_FROM_PARENT, index(), X11::detail::root_window_id(),
                      window.rect().pos.x, window.rect().pos.y, window.rect().size.x, window.rect().size.y,
                      // Draw border later
                      0, XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT,
                      mask, values);
    window::change_property(index(), window::prop::replace, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, std::span{constant::FRAME_CLASS_NAME});
    logger::debug("Window_frame -> created X11 frame {:#x}", index());

    xcb_reparent_window(X11::detail::conn(), window.index(), index(), 0, 0);

    xcb_map_window(X11::detail::conn(), index());
}

void Window_frame::update_rect() noexcept
{
    window::configure_rect(index(), this->rect());
}

Layout_frame::Layout_frame(Layout& layout)
    : ::Layout_frame(xcb_generate_id(X11::detail::conn()), layout)
{
    const uint32_t mask = XCB_CW_EVENT_MASK | XCB_CW_OVERRIDE_REDIRECT;
    const uint32_t values[] = { constant::FRAME_EVENT_MASK, 1 };
    xcb_create_window(X11::detail::conn(), XCB_COPY_FROM_PARENT, index(), X11::detail::root_window_id(),
                      layout.rect().pos.x, layout.rect().pos.y, layout.rect().size.x, layout.rect().size.y,
                      // Draw border later
                      0, XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT,
                      mask, values);
    window::change_property(index(), window::prop::replace, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, std::span{constant::FRAME_CLASS_NAME});
    logger::debug("Layout_frame -> created X11 frame {:#x}", index());

    xcb_map_window(X11::detail::conn(), index());
}

void Layout_frame::update_rect() noexcept
{
    window::configure_rect(index(), this->rect());
}

} // namespace X11