// Copyright (c) Tyler Veness. All Rights Reserved.

#include <stdint.h>

#include <atomic>
#include <cstddef>
#include <print>
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

    auto header = RawSocket::GetHeader(recvBuf);
    std::print("\nethernet [");
    std::print("src {:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}",
               header->h_source[0], header->h_source[1], header->h_source[2],
               header->h_source[3], header->h_source[4], header->h_source[5]);
    std::print(", dest {:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}",
               header->h_dest[0], header->h_dest[1], header->h_dest[2],
               header->h_dest[3], header->h_dest[4], header->h_dest[5]);
    std::println(", proto {}]", header->h_proto);

    auto payload = RawSocket::GetPayload(recvBuf);
    for (size_t i = 0; i < payload.size(); ++i) {
      if (i % 64 == 0) {
        std::print("\n   ");
      }
      std::print(" {:02X}", payload[i]);
    }
    std::println();
  }
}

int main(int argc, char* argv[]) {
  constexpr std::array<uint8_t, 6> destinationMac = {0x00, 0x00, 0x00,
                                                     0x00, 0x00, 0x00};

  constexpr const char* kDefaultInterface = "lo";

  const char* interfaceName;

  // Get interface name
  if (argc > 1) {
    interfaceName = argv[1];
  } else {
    interfaceName = kDefaultInterface;
  }

  RawSocket socket(interfaceName);
  socket.Bind(interfaceName);
  std::println("Bound raw socket to \"{}\"", interfaceName);

  std::array<char, 1024> sendbuf;
  size_t txLen = 0;

  // Packet data
  sendbuf[txLen++] = static_cast<char>(0xde);
  sendbuf[txLen++] = static_cast<char>(0xad);
  sendbuf[txLen++] = static_cast<char>(0xbe);
  sendbuf[txLen++] = static_cast<char>(0xef);

  isRunning = true;
  std::thread recvThread{recvFunc, std::ref(socket)};

  while (1) {
    // Send packet
    if (socket.SendTo(destinationMac, {sendbuf.data(), txLen}) < 0) {
      std::println("Send failed");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  isRunning = false;
  recvThread.join();
}
