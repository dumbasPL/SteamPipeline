#pragma once
#include "SteamPipeClient.h"
#include <winsock2.h>
#include <memory>

class SocketPipeline {
  SOCKET socket;
  std::shared_ptr<SteamPipeClient> steamPipe;
public:
  SocketPipeline(SOCKET socket, std::shared_ptr<SteamPipeClient> steamPipe);
  ~SocketPipeline();

  void Destroy();
  bool Read();
  bool Write();

  // NOTE: read and write here is the context for the socket, not the steam pipe
  virtual bool OnRead(uint8_t* packet, DWORD len) = 0;
  virtual bool OnWrite(uint8_t* packet, DWORD len) = 0;
  
  static void Start(std::shared_ptr<SocketPipeline> self);
};