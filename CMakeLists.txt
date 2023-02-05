cmake_minimum_required(VERSION 3.4)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
project(cubewm DESCRIPTION "A Window Manager")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "-g -Og -Wall -Winvalid-pch -Wno-reorder -pthread -fPIC -rdynamic")
else ()
    set(CMAKE_CXX_FLAGS "-Ofast -Wall -Winvalid-pch -Wno-reorder -pthread -fPIC -rdynamic")
endif ()

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

find_package(PkgConfig)
pkg_check_modules(DEPS REQUIRED xcb xcb-icccm xcb-keysyms xcb-xkb xcb-randr xcb-shape xcb-util xkbcommon xkbcommon-x11 xcb-cursor fmt)

include_directories(${DEPS_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${DEPS_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
