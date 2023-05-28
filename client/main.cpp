#include <SteamPipeServer.h>
#include <iostream>

int main(int, char**) {
  SteamPipeServer server("nezu");
  while (true) {
    auto connection = server.AcceptConnection();
    std::cout << "Accepted connection: " << (void*)connection.writePipeRead << std::endl;
  }
}
