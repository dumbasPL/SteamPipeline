#pragma once
#include <SocketPipeline.h>

namespace server {
  class ServerPipeline : public SocketPipeline {
  public:
    ServerPipeline(SOCKET socket, std::shared_ptr<SteamPipeClient> steamPipe) : SocketPipeline(socket, steamPipe) {};

    bool OnRead(uint8_t* packet, DWORD len);
    bool OnWrite(uint8_t* packet, DWORD len);
  };
}