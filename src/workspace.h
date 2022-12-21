#pragma once
#include "container.h"

class Monitor;

class Workspace : public Container
{
    Monitor& mon;

public:
    Workspace(Server& srv, Monitor& mon, uint32_t index);
    ~Workspace();
    
    uint32_t index;
    
    Workspace* get_workspace() override;
};
