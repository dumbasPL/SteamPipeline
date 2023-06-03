#include "client.h"
#include <iostream>
#include <format>

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cout << std::format("Usage: {} <pipe name> <host> <port>\n", argv[0]);
    return 1;
  }
  return client::run(argv[1], argv[2], argv[3]);
}
