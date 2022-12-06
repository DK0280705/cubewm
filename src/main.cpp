#include "logger.h"
#include "server.h"
#include <getopt.h>
#include <string>
#include <unistd.h>

int main(int argc, char* const argv[])
{
    static struct option options[] = {
        {"help", no_argument, 0, 'h'}
    };
    int option_index, opt = 0;

    while ((opt = getopt_long(argc, argv, "h", options, &option_index)) != -1) {
        switch (opt) {
        case 'h':
            // Print help
            Log::info("Still WIP");
            return 0;
        }
    }

    Log::info("Starting cubewm...");

    Server* srv = Server::init();
    if (!srv) return 1;

    try {
        srv->run();
    } catch (const std::exception& e) {
        Log::error(e.what());
    }

    return 0;
}
