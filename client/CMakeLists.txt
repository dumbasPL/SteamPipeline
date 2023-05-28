add_definitions(-DWIN32_LEAN_AND_MEAN)

file(GLOB_RECURSE SOURCES "*.cpp")
add_executable(SteamPipelineClient ${SOURCES})

target_link_libraries(SteamPipelineClient PUBLIC LibSteamPipeline)