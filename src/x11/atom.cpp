#include "atom.h"
#include "connection.h"
#include "ewmh.h"
#include "x11.h"

#include "../helper/memory.h"

#include <cstring>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>

namespace X11::atom {

#define xmacro(name) xcb_atom_t name = 0;
    ALL_ATOMS_XMACRO
    xcb_atom_t WM_SN = 0;
#undef xmacro

void init(const X11::Connection& conn)
{
#define xmacro(name) { name = by_name(conn, #name); }
    ALL_ATOMS_XMACRO;
    WM_SN = by_screen(conn, "WM");
#undef xmacro
}

auto by_name(const X11::Connection& conn, const char* name) -> xcb_atom_t
{
    auto reply = memory::c_own(xcb_intern_atom_reply(
        conn,
        xcb_intern_atom_unchecked(conn, false, strlen(name), name),
        nullptr));
    return reply->atom;
}

auto by_screen(const X11::Connection& conn, const char* base_name) -> xcb_atom_t
{
    auto name = memory::c_own(xcb_atom_name_by_screen(base_name, conn.scr_id()));
    return by_name(conn, name.get());
}

auto name(const X11::Connection& conn, const xcb_atom_t atom) -> std::string
{
    auto reply = memory::c_own<xcb_get_atom_name_reply_t>(
        xcb_get_atom_name_reply(
            conn,
            xcb_get_atom_name(conn, atom),
            nullptr));
    if (!reply) throw std::runtime_error("Failed to get atom name");
    return std::string(xcb_get_atom_name_name(reply.get()), xcb_get_atom_name_name_length(reply.get()));
}
}
