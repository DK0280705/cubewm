cmake_minimum_required(VERSION 3.4)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
project(cubewm DESCRIPTION "A Window Manager")

add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-unused-value -Wno-missing-field-initializers -Wno-narrowing)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

find_package(PkgConfig)
pkg_check_modules(DEPS REQUIRED xcb xcb-icccm xcb-keysyms xcb-xkb xcb-randr xcb-shape xcb-util xcb-cursor fmt libstartup-notification-1.0)

include_directories(${DEPS_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${DEPS_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
