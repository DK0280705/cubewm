cmake_minimum_required(VERSION 3.8)

enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build, options are: Debug Release."
        FORCE)
endif()

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "What the hell!? What are you doing?")
endif()

project(cubewm LANGUAGES CXX DESCRIPTION "A Window Manager" VERSION 0.0.1)

message("=== ${PROJECT_NAME} ${PROJECT_VERSION}")
message("-- Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

add_compile_options(
    -Wall
    -Wextra
    -fPIC
    -rdynamic
    -pthread
    -pedantic
)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

find_package(PkgConfig)
pkg_check_modules(DEPS REQUIRED gtk4 xcb xcb-icccm xcb-keysyms xcb-xkb xcb-randr xcb-shape xcb-util xkbcommon xkbcommon-x11 xcb-cursor fmt)

include_directories(${DEPS_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${DEPS_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

add_custom_target(debug
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug -G Ninja ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
)

add_custom_target(release
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release -G Ninja ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMENT "Switch CMAKE_BUILD_TYPE to Release"
)
