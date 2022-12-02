#include <string>
#include "logger.h"
#include "server.h"
#include <unistd.h>
#include <getopt.h>

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

    // Init Window Manager trough constructor
    Server srv;
    srv.run();

    return 0;
}
