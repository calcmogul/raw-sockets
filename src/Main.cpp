// Copyright (c) 2018 Tyler Veness. All Rights Reserved.

#include <iostream>

#include "RawSocket.hpp"

int main(int argc, char* argv[]) {
  constexpr std::array<uint8_t, 6> destinationMac = {0x00, 0x00, 0x00,
                                                     0x00, 0x00, 0x00};

  constexpr const char* kDefaultInterface = "eth0";

  std::string interfaceName;

  // Get interface name
  if (argc > 1) {
    interfaceName = argv[1];
  } else {
    interfaceName = kDefaultInterface;
  }

  RawSocket socket(interfaceName);

  char sendbuf[1024];
  int txLen = 0;

  // Packet data
  sendbuf[txLen++] = 0xde;
  sendbuf[txLen++] = 0xad;
  sendbuf[txLen++] = 0xbe;
  sendbuf[txLen++] = 0xef;

  // Send packet
  if (socket.SendTo(destinationMac, sendbuf, txLen) < 0) {
    std::cout << "Send failed\n";
  }
}
