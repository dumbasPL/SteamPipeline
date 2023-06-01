#include "server.h"
#include <windows.h>
#include <thread>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  if (fdwReason == DLL_PROCESS_ATTACH) {
    std::thread([hinstDLL](){
      FreeLibraryAndExitThread(hinstDLL, server::run());
    }).detach();
  }
  return TRUE;
}
 