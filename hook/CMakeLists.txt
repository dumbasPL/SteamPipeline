add_definitions(-DWIN32_LEAN_AND_MEAN)

set(SOURCES_MINHOOK 
  "minhook/buffer.c"
  "minhook/hook.c"
  "minhook/trampoline.c"
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(SOURCES_HDE "minhook/hde/hde64.c")
else()
  set(SOURCES_HDE "minhook/hde/hde32.c")
endif()

add_library(SteamPipeHook SHARED hook.cpp RTTI.cpp ${SOURCES_MINHOOK} ${SOURCES_HDE})
target_link_libraries(SteamPipeHook shlwapi)