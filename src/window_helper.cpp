#include "window_helper.h"
#include "helper.h"
#include "logger.h"
#include "workspace.h"
#include "window.h"

void place::operator()(Workspace* ws) const
{
    logger::debug("place: Visiting workspace");
    window->workspace(ws);
    if (ws->empty() || create_new) {
        // By default it's a horizontal container
        // Maybe i will add a config to change default container
        Layout_container* con = new Horizontal_container(ws);
        ws->add(con);
        ws->update_rect(ws->rect());
        con->accept(place(window));
    } else {
        Layout_container* con = (*ws)[0];
        assert_debug(con, "Not a layout container");
        con->accept(place(window));
    }
}

void place::operator()(Layout_container* con) const
{
    logger::debug("place: Visiting layout container");
    con->add(window);
    con->update_rect(con->rect());
}

void purge::operator()(Layout_container* con) const
{
    con->remove(window);
    if (con->empty()) {
        // OK, let the UB kicks in
        auto* parent = con->parent();
        parent->remove(con);
        delete con;
        parent->update_rect(parent->rect());
    } else {
        con->update_rect(con->rect());
    }
}
