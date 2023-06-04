#pragma once
#include <SocketPipeline.h>

namespace client {
  class ClientPipeline : public SocketPipeline {
  public:
    ClientPipeline(SOCKET socket, std::shared_ptr<SteamPipeClient> steamPipe) : SocketPipeline(socket, steamPipe) {};

    bool OnRead(uint8_t* packet, DWORD len);
    bool OnWrite(uint8_t* packet, DWORD len);
  private:
    uint8_t lastPacketType = 0;
  };
}