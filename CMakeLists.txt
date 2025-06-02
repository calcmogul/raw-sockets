cmake_minimum_required(VERSION 3.21)

# Set default build type to release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# Generate compile_commands.json by default
if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS "YES" CACHE STRING "" FORCE)
endif()

project(raw-sockets)

file(GLOB_RECURSE cpp_src src/*.cpp)
set_property(SOURCE ${cpp_src} PROPERTY COMPILE_FLAGS "-Wno-unused")
add_executable(raw-sockets ${cpp_src})

target_compile_options(raw-sockets PRIVATE -Wall -Wextra -pedantic)
target_compile_features(raw-sockets PUBLIC cxx_std_23)
target_include_directories(raw-sockets PRIVATE src)
