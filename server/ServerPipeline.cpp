#include "ServerPipeline.h"
#include <cassert>
#include <iostream>

#pragma pack(push, 1)
struct Base {
  uint8_t type;
};


struct ClientHello : public Base {
  uint32_t success;
  uint32_t processId;
  uint32_t threadId;
};
#pragma pack(pop)

// messages coming from a client
bool server::ServerPipeline::OnRead(uint8_t* packet, DWORD len) {
  uint8_t type = packet[0];

  switch (type) {
  case 9: { // hello from the game
    assert(len == sizeof(ClientHello));
    ClientHello* hello = (ClientHello*)packet;

    // original process might not exist on this machine, replace it with our own
    hello->processId = GetCurrentProcessId();
    hello->threadId = GetCurrentThreadId();
    std::cout << "Client hello replaced" << std::endl;
    break;
  }
  }

  return true;
}

// messages coming from steam
bool server::ServerPipeline::OnWrite(uint8_t* packet, DWORD len) {
  return true;
}
