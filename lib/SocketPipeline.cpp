#include "SocketPipeline.h"
#include <iostream>
#include <array>
#include <thread>

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

bool SocketPipeline::Read() {
  DWORD networkedSize = 0;
  if (int res = recv(socket, (char*)&networkedSize, sizeof(networkedSize), MSG_WAITALL); res != sizeof(networkedSize)) {
    std::cout << "Failed to receive message size: " << res << std::endl;
    return false;
  }

  // network byte order to host byte order
  DWORD messageSize = ntohl(networkedSize);
  std::vector<uint8_t> data(messageSize);

  if (int res = recv(socket, (char*)data.data(), messageSize, MSG_WAITALL); res != data.size()) {
    std::cout << "Failed to receive message: " << res << std::endl;
    return false;
  }

  if (!steamPipe->Write(data)) {
    std::cout << "Failed to write message to pipe" << std::endl;
    return false;
  }

  return true;
}

bool SocketPipeline::Write() {
  std::vector<uint8_t> data;
  if (!steamPipe->Read(data)) {
    std::cout << "Failed to read message from pipe" << std::endl;
    return false;
  }

  // host byte order to network byte order
  DWORD networkedSize = htonl(data.size());
  if (int res = send(socket, (char*)&networkedSize, sizeof(networkedSize), 0); res != sizeof(networkedSize)) {
    std::cout << "Failed to send message size: " << res << std::endl;
    return false;
  }

  if (int res = send(socket, (char*)data.data(), data.size(), 0); res != data.size()) {
    std::cout << "Failed to send message: " << res << std::endl;
    return false;
  }

  return true;
}

void SocketPipeline::Start(std::shared_ptr<SocketPipeline> self) {
  std::thread([self](){
    while (self->Write()) {}
    self->Destroy();
  }).detach();

  std::thread([self](){
    while (self->Read()) {}
    self->Destroy();
  }).detach();
}