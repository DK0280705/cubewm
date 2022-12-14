#include "root.h"
#include "container.h"
#include "dockarea.h"
#include "output.h"
#include "server.h"
#include "workspace.h"
#include <stdlib.h>
#include <xcb/randr.h>
#include <xcb/xcb.h>

Root::Root(Server* srv) : Container(srv)
{
    type = Container::CT_ROOT;
}

Root* Root::init(Server* srv)
{
    static Root root(srv);
    if (!root._get_monitors()) return nullptr;
    return &root;
}

Output& Root::create_output(const std::string& name, Rect rect)
{
    bool    reuse;
    Output* out = nullptr;

    for (const auto& o : children) {
        if (o->name == name) {
            out   = dynamic_cast<Output*>(o);
            reuse = true;
            break;
        }
    }

    if (reuse) {
        out       = new Output(srv_);
        out->name = name;
    }
    out->rect = rect;

    if (reuse) return *out;

    Dockarea* tda = new Dockarea(srv_);
    tda->name     = "topdock";
    Dockarea* bda = new Dockarea(srv_);
    bda->name     = "bottomdock";

    tda->attach_to(*out);
    bda->attach_to(*out);

    return *out;
}

bool Root::_get_monitors()
{
    if (!srv_->randr_support) {
    } else {
        xcb_generic_error_t*            err;
        xcb_randr_get_monitors_reply_t* monitors = xcb_randr_get_monitors_reply(
            srv_->conn, xcb_randr_get_monitors(srv_->conn, srv_->screen->root, true), &err);
        if (err) {
            free(err);
            return false;
        }
    }
    return true;
}

Root::~Root() {}
