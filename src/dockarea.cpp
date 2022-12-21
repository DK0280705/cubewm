#include "dockarea.h"

Dockarea::Dockarea(Server& srv, Monitor& mon)
    : Container(srv)
    , mon(mon)
{
    type = Container::CT_DOCKAREA;
}
