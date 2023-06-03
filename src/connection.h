#pragma once
/**
 * Define X11 server connection
 * Maybe extended with Wayland compositor
 * I'll give it a try
 */
#include "helper.h"
#include "x11/connection.h"
#ifdef USE_WAYLAND
#include "wayland/connection.h"
#endif

class Connection : public X11::Connection
#ifdef USE_WAYLAND
                 , public Wayland::Connection
#endif
                 , public Init_once<Connection>
{
public:
    Connection()
        : X11::Connection()
#ifdef USE_WAYLAND
        , Wayland::Connection()
#endif
    {}
};

uint32_t root_window_id(const X11::Connection& conn) noexcept;