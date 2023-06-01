add_definitions(-DWIN32_LEAN_AND_MEAN)

add_executable(SteamPipelineServer server.cpp main.cpp)
target_link_libraries(SteamPipelineServer PUBLIC LibSteamPipeline ws2_32)

add_library(SteamPipelineServerDll SHARED server.cpp dllmain.cpp)
target_link_libraries(SteamPipelineServerDll PUBLIC LibSteamPipeline ws2_32)
