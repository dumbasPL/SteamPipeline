#pragma once
#include "SteamSharedMemory.h"

class SteamPipeClient
{
public:
  HANDLE syncRead;
  HANDLE syncWrite;

  HANDLE readPipeRead = INVALID_HANDLE_VALUE;
  HANDLE readPipeWrite = INVALID_HANDLE_VALUE;

  HANDLE writePipeRead = INVALID_HANDLE_VALUE;
  HANDLE writePipeWrite = INVALID_HANDLE_VALUE;

  SteamPipeClient();
  SteamPipeClient(HANDLE syncRead);
  SteamPipeClient(HANDLE syncRead, HANDLE syncWrite);
  ~SteamPipeClient();

  bool Connect(const char *ipcName = "Steam3Master");
};