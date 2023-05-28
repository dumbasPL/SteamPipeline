#include <SteamPipeClient.h>
#include <iostream>

int main(int, char**) {
  SteamPipeClient client;
  std::cout << "Connected: " << client.Connect("nezu") << std::endl;
}