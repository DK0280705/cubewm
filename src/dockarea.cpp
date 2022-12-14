#include "dockarea.h"

Dockarea::Dockarea(Server* srv)
    : Container(srv)
{
    type = Container::CT_DOCKAREA;
}
