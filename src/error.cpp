#include "error.h"
#include <stdexcept>
#include <fmt/format.h>

Init_error::Init_error(const char* type_name)
    : std::runtime_error(fmt::format("You called {}::init more than once!", type_name))
{
}
