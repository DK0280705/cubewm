#include "logger.h"
#include <mutex>

namespace logger
{
static std::mutex _mutex;

void print_line(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    fmt::print("{}\n", msg);
}
}
