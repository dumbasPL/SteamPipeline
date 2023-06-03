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
  
  static void Start(std::shared_ptr<SocketPipeline> self);
};