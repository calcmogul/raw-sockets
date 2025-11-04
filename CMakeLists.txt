cmake_minimum_required(VERSION 3.28)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(raw-sockets)

file(GLOB_RECURSE cpp_src src/*.cpp)
add_executable(raw-sockets ${cpp_src})

target_compile_options(raw-sockets PRIVATE -Wall -Wextra -pedantic)
target_compile_features(raw-sockets PUBLIC cxx_std_23)
target_include_directories(raw-sockets PRIVATE src)
