#include "logger.h"
#include <mutex>
namespace Log
{
static std::mutex mutex;

void send_msg(std::ostream &o, const std::string& msg)
{
    std::lock_guard<std::mutex> lock(mutex);
    fmt::print(o, "{}\n", msg); 
}
}
