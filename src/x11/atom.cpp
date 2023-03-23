#include "atom.h"
#include "x11.h"
#include "../helper.h"
#include "../connection.h"
#include <cstring>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>

namespace X11::atom {

#define xmacro(name) xcb_atom_t name = 0;
    ALL_ATOMS_XMACRO
    xcb_atom_t WM_SN = 0;
#undef xmacro

void init()
{
#define xmacro(name) { name = by_name(#name); }
    ALL_ATOMS_XMACRO;
    WM_SN = by_screen("WM");
#undef xmacro
}

xcb_atom_t by_name(const char* name)
{
    auto reply = memory::c_own<xcb_intern_atom_reply_t>(
        xcb_intern_atom_reply(
            X11::_conn(),
            xcb_intern_atom_unchecked(X11::_conn(), false, strlen(name), name),
            nullptr));

    return reply->atom;
}

xcb_atom_t by_screen(const char* base_name)
{
    char* name = xcb_atom_name_by_screen(base_name, X11::_conn().scr_id());

    const xcb_atom_t atom = by_name(name);
    free(name);

    return atom;
}

std::string name(const xcb_atom_t atom)
{
    auto reply = memory::c_own<xcb_get_atom_name_reply_t>(
        xcb_get_atom_name_reply(
            X11::_conn(),
            xcb_get_atom_name(X11::_conn(), atom),
            NULL));

    return xcb_get_atom_name_name(reply.get());
}
}
