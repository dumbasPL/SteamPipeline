#include "client.h"
#include <SteamPipeServer.h>
#include <SocketPipeline.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>

int client::run(const char* name, const char* host, const char* port) {
  WSADATA wsaData;
  if (int error = WSAStartup(MAKEWORD(2,2), &wsaData)) {
    std::cout << "WSAStartup failed: " << error << std::endl;
    return 1;
  }

  addrinfo hints = {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  SteamPipeServer server(name);
  while (true) {
    const std::shared_ptr<SteamPipeClient> steamPipeClient = server.AcceptConnection();
    if (!steamPipeClient) {
      std::cout << "AcceptConnection failed" << std::endl;
      continue;
    }

    std::cout << "Accepted connection from PID: " << steamPipeClient->remoteProcessId << std::endl;

    addrinfo* result = nullptr;
    if (int error = getaddrinfo(host, port, &hints, &result)) {
      std::cout << "getaddrinfo failed: " << error << std::endl;
      continue;
    }

    SOCKET connectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (connectSocket == INVALID_SOCKET) {
      std::cout << "creating socket failed: " << WSAGetLastError() << std::endl;
      freeaddrinfo(result);
      continue;
    }

    if (connect(connectSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
      std::cout << "socket connect failed: " << WSAGetLastError() << std::endl;
      closesocket(connectSocket);
      freeaddrinfo(result);
      continue;
    }

    // FIXME: try next address if connect fails
    freeaddrinfo(result);

    const auto pipeline = std::make_shared<SocketPipeline>(connectSocket, steamPipeClient);
    SocketPipeline::Start(pipeline);
  }
}
