#include <steam/steam_api.h>
#include <windows.h>
#include <iostream>

__declspec(dllimport) void DummyServerFunc();
__declspec(dllimport) void DummyClientFunc();

int main(int, char**) {
  // this is here so that we can easily inject dlls with Koaloader ;)
  LoadLibraryA("d3d9.dll");

  // force our dlls to be loaded
  DummyServerFunc();
  DummyClientFunc();

  if (!getenv("steam_master_ipc_name_override"))
    SetEnvironmentVariableA("steam_master_ipc_name_override", "nezu");

  if (!SteamAPI_Init()) {
    std::cout << "Failed to initialize Steam API" << std::endl;
    return 1;
  }
  // SteamAPI_RunCallbacks();
  std::cout << "SteamAPI_IsSteamRunning: " << SteamAPI_IsSteamRunning() << std::endl;
  std::cout << "SteamFriends()->GetPersonaName(): " << SteamFriends()->GetPersonaName() << std::endl;
  return 0;
}