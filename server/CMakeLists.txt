add_definitions(-DWIN32_LEAN_AND_MEAN)

set(SOURCES_SERVER
  "server.cpp"
  "ServerPipeline.cpp"
)

set(LIBS_SERVER
  "LibSteamPipeline"
  "ws2_32"
)

add_executable(SteamPipelineServer main.cpp ${SOURCES_SERVER})
target_link_libraries(SteamPipelineServer PUBLIC ${LIBS_SERVER})

add_library(SteamPipelineServerDll SHARED dllmain.cpp ${SOURCES_SERVER})
target_link_libraries(SteamPipelineServerDll PUBLIC ${LIBS_SERVER})
