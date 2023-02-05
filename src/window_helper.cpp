#include "window_helper.h"
#include "workspace.h"
#include "window.h"

void Place_window::operator ()(Manager<Workspace>& manager) const
{
    Workspace* ws = manager.at(window->workspace()->index());

    Container* focused     = ws->focused();
    Container* updated_con = nullptr;

    if (focused) {
        Node<Container>* parent = focused->parent();
        parent->add(window);
        updated_con = parent;

    } else if (!ws->empty()) {
        Layout_container* lcon = dynamic_cast<Layout_container*>(*ws->begin());
        lcon->add(window);
        updated_con = lcon;

    } else {
        // By default it's a horizontal container
        // Maybe i will add a config to change default container
        Horizontal_container* hcon = new Horizontal_container(ws);
        hcon->add(window);
        ws->add(hcon);
        updated_con = ws;
    }

    updated_con->update_rect(updated_con->rect());
}

void Purge_window::operator ()(Manager<Workspace>& manager) const
{
    Workspace* ws = manager.at(window->workspace()->index());

    Node<Container>* parent      = window->parent();
    Node<Container>* updated_con = nullptr;

    parent->remove(window);

    if (parent->size() == 0) {
        updated_con = parent->parent();
        updated_con->remove(parent);
        delete parent;

        if (ws->focused() == window)
            ws->focused(updated_con);
 
    } else {
        updated_con = parent;
        if (ws->focused() == window)
            ws->focused(*std::prev(parent->end(), 1));
    }

    updated_con->update_rect(updated_con->rect());
}
