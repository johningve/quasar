cmake_minimum_required(VERSION 3.25)

project(quasar)

set(CMAKE_CXX_STANDARD 20)

find_path(BOTAN_INCLUDE_DIR botan/botan.h REQUIRED)
find_library(BOTAN_LIBRARY botan-2 REQUIRED)

find_package(Catch2 CONFIG REQUIRED)
find_package(cppzmq CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_subdirectory(src)