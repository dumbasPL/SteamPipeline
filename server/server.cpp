#include "server.h"
#include <SteamPipeClient.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
#include "ServerPipeline.h"

int server::run(const char* name, const char* port) {
  WSADATA wsaData;
  if (int error = WSAStartup(MAKEWORD(2,2), &wsaData)) {
    std::cout << "WSAStartup failed: " << error << std::endl;
    return 1;
  }

  addrinfo hints = {};
  hints.ai_family = AF_INET; // FIXME: ipv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  addrinfo* result = nullptr;
  if (int error = getaddrinfo(NULL, port, &hints, &result)) {
    std::cout << "getaddrinfo failed: " << error << std::endl;
    WSACleanup();
    return 1;
  }

  SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (listenSocket == INVALID_SOCKET) {
    std::cout << "creating socket failed: " << WSAGetLastError() << std::endl;
    freeaddrinfo(result);
    WSACleanup();
    return 1;
  }

  if (bind(listenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
    std::cout << "socket bind failed: " << WSAGetLastError() << std::endl;
    freeaddrinfo(result);
    closesocket(listenSocket);
    WSACleanup();
    return 1;
  }

  freeaddrinfo(result);

  if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
    std::cout << "socket listen failed: " << WSAGetLastError() << std::endl;
    closesocket(listenSocket);
    WSACleanup();
    return 1;
  }

  while (true) {
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrSize);

    if (clientSocket == INVALID_SOCKET) {
      std::cout << "socket accept failed: " << WSAGetLastError() << std::endl;
      closesocket(listenSocket);
      WSACleanup();
      return 1;
    }

    BOOL nodelay = TRUE;
    if (setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay)) == SOCKET_ERROR) {
      std::cout << "setsockopt TCP_NODELAY failed: " << WSAGetLastError() << std::endl;
    }

    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
    std::cout << "Incoming connection from: " << clientIp << std::endl;

    auto steamPipeClient = std::make_shared<SteamPipeClient>();
    if (!steamPipeClient->Connect(name)) {
      std::cout << "Failed to connect to SteamPipeServer" << std::endl;
      closesocket(clientSocket);
      continue;
    }
    std::cout << "Connected to SteamPipeServer with PID: " << steamPipeClient->remoteProcessId << std::endl;

    const auto pipeline = std::make_shared<ServerPipeline>(clientSocket, steamPipeClient);
    SocketPipeline::Start(pipeline);
  }
}
