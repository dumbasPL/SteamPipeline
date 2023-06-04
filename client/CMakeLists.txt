add_definitions(-DWIN32_LEAN_AND_MEAN)

set(SOURCES_CLIENT
  "client.cpp"
  "ClientPipeline.cpp"
)

set(LIBS_CLIENT
  "LibSteamPipeline"
  "ws2_32"
)

add_executable(SteamPipelineClient main.cpp ${SOURCES_CLIENT})
target_link_libraries(SteamPipelineClient PUBLIC ${LIBS_CLIENT})

add_library(SteamPipelineClientDll SHARED dllmain.cpp ${SOURCES_CLIENT})
target_link_libraries(SteamPipelineClientDll PUBLIC ${LIBS_CLIENT})