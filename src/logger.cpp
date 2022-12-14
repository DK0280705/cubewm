#include "logger.h"
#include <mutex>
namespace Log
{
static std::mutex mutex;

void print(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(mutex);
    fmt::print("{}\n", msg); 
}
}
