#pragma once
#include "SteamPipeClient.h"
#include <winsock2.h>
#include <memory>
#include <thread>
#include <mutex>

class SocketPipeline {
  SOCKET socket;
  std::shared_ptr<SteamPipeClient> steamPipe;

  std::thread readThread;
  std::thread writeThread;

  std::mutex mtx;
public:
  SocketPipeline(SOCKET socket, std::shared_ptr<SteamPipeClient> steamPipe);
  ~SocketPipeline();

  void Destroy();
  void Read();
  void Write();
  
  static void Start(std::shared_ptr<SocketPipeline> self);
};