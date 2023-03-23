#pragma once
#include "../server.h"
#include "../helper.h"

namespace X11 {

class Server : public ::Server
             , public Init_once<Server>
{
public:
    Server(State& state);

    void start() override;
    void stop()  override;

private:
    void _main_loop();
};

}
