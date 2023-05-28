add_definitions(-DWIN32_LEAN_AND_MEAN)

file(GLOB_RECURSE SOURCES "*.cpp")
add_library(LibSteamPipeline STATIC ${SOURCES})

target_include_directories(LibSteamPipeline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})