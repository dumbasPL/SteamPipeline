add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

link_directories(lib include/steam/lib/win32)

add_executable(SteamPipelineDemo demo.cpp)
target_include_directories(SteamPipelineDemo PUBLIC include)
target_link_libraries(SteamPipelineDemo PUBLIC steam_api SteamPipelineClientDll SteamPipelineServerDll)

add_custom_command(TARGET SteamPipelineDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/steam_api.dll"
        $<TARGET_FILE_DIR:SteamPipelineDemo>
)

add_custom_command(TARGET SteamPipelineDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/steam_appid.txt"
        $<TARGET_FILE_DIR:SteamPipelineDemo>
)