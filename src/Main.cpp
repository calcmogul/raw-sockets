// Copyright (c) 2018 Tyler Veness. All Rights Reserved.

#include <stdint.h>

#include <atomic>
#include <functional>
#include <iostream>
#include <thread>

#include "RawSocket.hpp"
#include "StringView.hpp"

std::atomic<bool> isRunning{false};

void recvFunc(RawSocket& socket) {
  char recvbuf[65536];

  while (isRunning) {
    StringView recvView{recvbuf, 65536};

    // Receive packet
    recvView = socket.RecvFrom(recvView);
    if (recvView.str == nullptr) {
      continue;
    }

    // Parse ethernet header
    RawSocket::ParseHeader(recvView);

    // Get payload
    recvView = RawSocket::GetPayload(recvView);
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

  StringView sendView{sendbuf, txLen};

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
