#pragma once
#include <chrono>
#include "SteamSharedMemory.h"
#include "SteamPipeClient.h"

class SteamPipeServer
{
  HANDLE fileMap;
  HANDLE event;
  SteamSharedMemory* sharedMem;

  std::chrono::steady_clock::time_point lastAccept;
public:
  SteamPipeServer(const char* ipcName = "Steam3Master");
  ~SteamPipeServer();

  SteamPipeClient AcceptConnection();
private:
  std::unique_ptr<SteamPipeClient> AcceptConnectionInternal();
  void Reset();
};