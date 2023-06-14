#pragma once
#include "../server.h"
#include "../helper/mixins.h"

class Connection;

namespace X11 {

// X11 Server is a window manager instance.
// Creates window manager client.
class Server : public ::Server
             , public helper::Init_once<Server>
{
public:
    explicit Server(::Connection& conn);
    static auto init(::Connection& conn) -> Server&;

    void start() override;
    void stop()  override;
};

}
