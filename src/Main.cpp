// Copyright (c) 2018-2020 Tyler Veness. All Rights Reserved.

#include <stdint.h>

#include <atomic>
#include <iostream>
#include <string_view>
#include <thread>

#include "RawSocket.hpp"

std::atomic<bool> isRunning{false};

void recvFunc(RawSocket& socket) {
  std::array<char, 65536> recvBuf;

  while (isRunning) {
    // Receive packet
    int readBytes = socket.RecvFrom(recvBuf);
    if (readBytes <= 0) {
      continue;
    }

    // Parse ethernet header
    RawSocket::ParseHeader(std::string_view{recvBuf.data(), recvBuf.size()});

    // Get payload
    std::string_view recvView = RawSocket::GetPayload(recvView);
  }
}

int main(int argc, char* argv[]) {
  constexpr std::array<uint8_t, 6> destinationMac = {0x00, 0x00, 0x00,
                                                     0x00, 0x00, 0x00};

  constexpr const char* kDefaultInterface = "lo";

  std::string interfaceName;

  // Get interface name
  if (argc > 1) {
    interfaceName = argv[1];
  } else {
    interfaceName = kDefaultInterface;
  }

  RawSocket socket(interfaceName);
  socket.Bind(interfaceName);
  std::cout << "Bound raw socket to \"" << interfaceName << "\"\n";

  char sendbuf[1024];
  size_t txLen = 0;

  // Packet data
  sendbuf[txLen++] = 0xde;
  sendbuf[txLen++] = 0xad;
  sendbuf[txLen++] = 0xbe;
  sendbuf[txLen++] = 0xef;

  std::string_view sendView{sendbuf, txLen};

  isRunning = true;
  std::thread recvThread{recvFunc, std::ref(socket)};

  while (1) {
    // Send packet
    if (socket.SendTo(destinationMac, sendView) < 0) {
      std::cout << "Send failed\n";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  isRunning = false;
  recvThread.join();
}
