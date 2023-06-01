add_definitions(-DWIN32_LEAN_AND_MEAN)

add_executable(SteamPipelineClient client.cpp main.cpp)
target_link_libraries(SteamPipelineClient PUBLIC LibSteamPipeline ws2_32)

add_library(SteamPipelineClientDll SHARED client.cpp dllmain.cpp)
target_link_libraries(SteamPipelineClientDll PUBLIC LibSteamPipeline ws2_32)