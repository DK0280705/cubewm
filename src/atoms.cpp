#include "atoms.h"
#include "connection.h"
#include "error.h"
#include <cstddef>
#include <cstring>
#include <memory>
#include <xcb/xproto.h>

extern "C" {
#include <xcb/xcb_atom.h>
}

namespace Atom
{
// The atoms "should" be const, but actually, we don't know the value yet
#define xmacro(name) xcb_atom_t name = 0;
ALL_ATOMS_XMACRO; // NOLINT
xmacro(WM_SN);    // NOLINT
#undef xmacro

// Not to be afraid of data races
// It should be safe as it's constant
// The connection will be closed at the end of the program
// as it is initialized on static variable.
static const Connection* conn = nullptr; // NOLINT

void init(const Connection& conn)
{
    if (Atom::conn) throw Init_error("Atom");
    Atom::conn = &conn;

#define xmacro(name)                                                           \
    {                                                                          \
        Atom::name = Atom::by_name(#name);                                     \
    }
    ALL_ATOMS_XMACRO;
#undef xmacro
    Atom::WM_SN = by_screen("WM");
}

xcb_atom_t by_name(const char* name)
{
    auto reply = std::unique_ptr<xcb_intern_atom_reply_t, void(*)(void*)>(
        xcb_intern_atom_reply(
            *conn,
            xcb_intern_atom_unchecked(*conn, false, strlen(name), name),
            nullptr),
        free);

    return reply->atom;
}

xcb_atom_t by_screen(const char* base_name)
{
    char* name = xcb_atom_name_by_screen(base_name, conn->scr_id());

    const xcb_atom_t atom = by_name(name);
    free(name);

    return atom;
}

std::string name(const xcb_atom_t atom)
{
    auto reply = std::unique_ptr<xcb_get_atom_name_reply_t, void(*)(void*)>(
        xcb_get_atom_name_reply(
            *conn,
            xcb_get_atom_name(*conn, atom),
            NULL),
        free);

    return xcb_get_atom_name_name(reply.get());
}

} // namespace Atom
