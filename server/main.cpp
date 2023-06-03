#include "server.h"
#include <iostream>
#include <format>

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout << std::format("Usage: {} <pipe name> <port>\n", argv[0]);
    return 1;
  }
  return server::run(argv[1], argv[2]);
}