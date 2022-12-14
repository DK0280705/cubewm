#include "output.h"

Output::Output(Server* srv)
    : Container(srv)
{
    type = Container::CT_OUTPUT;
}
