#include "ClientPipeline.h"
#include <cassert>
#include <iostream>

#pragma pack(push, 1)
struct ServerHello {
  uint32_t processId;
  uint32_t threadId;
  uint32_t seed;
};
#pragma pack(pop)

// messages coming from server
bool client::ClientPipeline::OnRead(uint8_t* packet, DWORD len) {
  // NOTE: responses don't always have message IDs

  // process response based on last request
  switch (lastPacketType) {
    case 9: { // server hello (client hello response)
      assert(len == sizeof(ServerHello));
      ServerHello* hello = (ServerHello*)packet;

      // replace process and thread IDs with our own
      hello->processId = GetCurrentProcessId();
      hello->threadId = GetCurrentThreadId();
      std::cout << "Server hello replaced" << std::endl;
      break;
    }
  }

  return true;
}

// messages coming from the game
bool client::ClientPipeline::OnWrite(uint8_t* packet, DWORD len) {
  uint8_t type = packet[0];

  // processing here

  lastPacketType = type;
  return true;
}
