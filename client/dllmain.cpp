#include "client.h"
#include <windows.h>
#include <thread>

__declspec(dllexport) void DummyClientFunc() { }

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  if (fdwReason == DLL_PROCESS_ATTACH) {
    std::thread([hinstDLL](){
      FreeLibraryAndExitThread(hinstDLL, client::run("nezu", "127.0.0.1", "11728"));
    }).detach();
  }
  return TRUE;
}
 