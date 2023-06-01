#include "SocketPipeline.h"
#include <iostream>
#include <array>

SocketPipeline::SocketPipeline(SOCKET socket, std::shared_ptr<SteamPipeClient> steamPipe) 
  : socket(socket), steamPipe(steamPipe) {}

SocketPipeline::~SocketPipeline() {
  std::cout << "SocketPipeline destroyed" << std::endl;
  Destroy();
}

void SocketPipeline::Destroy() {
  closesocket(socket);
  steamPipe->Destroy();
}

void SocketPipeline::Read() {
  std::array<char, 0x2000> buffer; // default pipe buffer size
  while (true) {
    DWORD networkedSize = 0;
    if (int res = recv(socket, (char*)&networkedSize, sizeof(networkedSize), MSG_WAITALL); res != sizeof(networkedSize)) {
      std::cout << "Failed to receive message size: " << res << std::endl;
      Destroy();
      return;
    }

    std::lock_guard lock(mtx);

    // network byte order to host byte order
    DWORD messageSize = ntohl(networkedSize);

    if (messageSize > buffer.size()) {
      std::cout << "Message too large: " << messageSize << std::endl;
      Destroy();
      return;
    }

    // receive the message
    if (int res = recv(socket, buffer.data(), messageSize, MSG_WAITALL); res != messageSize) {
      std::cout << "Failed to receive message: " << res << std::endl;
      Destroy();
      return;
    }

    // write the message to the pipe
    if (!steamPipe->Write(buffer.data(), messageSize)) {
      std::cout << "Failed to write message to pipe" << std::endl;
      Destroy();
      return;
    }

    std::cout << "Read: Sent message of size " << messageSize << std::endl;
  }
}

void SocketPipeline::Write() {
  constexpr auto bufferSize = 0x2000; // default pipe buffer size
  constexpr auto sizeSize = sizeof(DWORD);
  std::array<char, bufferSize + sizeSize> buffer;
  while (true) {
    DWORD messageSize = 0;
    if (!steamPipe->Read(buffer.data() + sizeSize, bufferSize, &messageSize)) {
      std::cout << "Failed to read message from pipe" << std::endl;
      Destroy();
      return;
    }

    std::lock_guard lock(mtx);

    // host byte order to network byte order
    DWORD networkedSize = htonl(messageSize);
    memcpy(buffer.data(), &networkedSize, sizeof(networkedSize));

    const auto totalSize = sizeSize + messageSize;
    if (int res = send(socket, buffer.data(), totalSize, 0); res != totalSize) {
      std::cout << "Failed to send message: " << res << std::endl;
      Destroy();
      return;
    }

    std::cout << "Write: Sent message of size " << messageSize << std::endl;
  }
}

void SocketPipeline::Start(std::shared_ptr<SocketPipeline> self) {
  self->readThread = std::thread(&SocketPipeline::Read, self);
  self->writeThread = std::thread(&SocketPipeline::Write, self);
}