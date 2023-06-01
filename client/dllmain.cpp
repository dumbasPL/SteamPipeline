#include "client.h"
#include <windows.h>
#include <thread>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  if (fdwReason == DLL_PROCESS_ATTACH) {
    std::thread([hinstDLL](){
      FreeLibraryAndExitThread(hinstDLL, client::run());
    }).detach();
  }
  return TRUE;
}
 