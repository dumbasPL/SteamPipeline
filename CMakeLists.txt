cmake_minimum_required(VERSION 3.0.0)
project(SteamPipeline VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(lib)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(demo)
add_subdirectory(hook)
