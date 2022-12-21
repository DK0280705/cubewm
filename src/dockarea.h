#pragma once
#include "container.h"

class Monitor;

class Dockarea : public Container
{
    Monitor& mon;

public:
    Dockarea(Server& srv, Monitor& mon);
};
