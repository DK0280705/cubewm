#ifdef USE_WAYLAND
#include "wayland/server.h"
#else
#include "x11/server.h"
#endif

#include "connection.h"
#include "config.h"
#include "state.h"
#include "logger.h"
#include <array>
#include <getopt.h>

Timestamp State::_timestamp{}; // Avoid linker error.

static bool _parse_arguments(int argc, char* const argv[])
{
    static constexpr std::array<option, 3> options{{
         {"help", no_argument, 0, 'h'},
         {"replace", no_argument, 0, 'r'},
         {"use-xinerama", no_argument, 0, 'x'},
    }};

    int opt_index = 0;
    int opt = 0;

    while ((opt = getopt_long(argc, argv, "hrx",
                              options.data(), &opt_index))
            != -1) {
        switch (opt) {
        case 'h':
            logger::info("Still WIP");
            return false;
        case 'r':
            config::replace_wm = true;
            break;
        case 'x':
            config::xinerama_enabled = true;
            break;
        default:
            logger::error("Unrecognized options");
            return false;
        }
    }

    return true;
}

int main(int argc, char* const argv[])
{
    if (!_parse_arguments(argc, argv))
        return 0;

    logger::info("Starting cubewm");

    try {
        Connection& conn = Connection::init();

        State& state = State::init(conn);

#ifdef USE_WAYLAND
        using Server_type = typename Wayland::Server;
#else
        using Server_type = typename X11::Server;
#endif

        state.init_server<Server_type>(state);
        Server& srv = state.server();

        srv.start();
    } catch (const std::runtime_error& err) {
        logger::error("Runtime Error: {}", err.what());
        return 1;
    }

    return 0;
}
