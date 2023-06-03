#pragma once
#include "SteamSharedMemory.h"
#include <vector>

class SteamPipeClient {
public:
  HANDLE syncRead;
  HANDLE syncWrite;

  HANDLE readPipeRead = INVALID_HANDLE_VALUE;
  HANDLE readPipeWrite = INVALID_HANDLE_VALUE;

  HANDLE writePipeRead = INVALID_HANDLE_VALUE;
  HANDLE writePipeWrite = INVALID_HANDLE_VALUE;

  int remoteProcessId = -1;

  SteamPipeClient();
  SteamPipeClient(HANDLE syncRead);
  SteamPipeClient(HANDLE syncRead, HANDLE syncWrite);
  ~SteamPipeClient();

  bool Connect(const char *ipcName = "Steam3Master");
  void Destroy();

  bool ReadInternal(std::vector<uint8_t> &data);
  bool Read(std::vector<uint8_t> &data);
  bool Write(std::vector<uint8_t> &data);
};