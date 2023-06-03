#pragma once
#include "../server.h"
#include "../helper.h"

class Connection;

namespace X11 {

// X11 Server is a window manager instance.
// Creates window manager client.
class Server : public ::Server
             , public Init_once<Server>
{
public:
    Server(::Connection& conn);
    static Server& init(::Connection& conn);

    void start() override;
    void stop()  override;
};

}
