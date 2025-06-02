// Copyright (c) Tyler Veness. All Rights Reserved.

#include <stdint.h>

#include <atomic>
#include <cstddef>
#include <print>
#include <thread>

#include "raw_socket.hpp"

std::atomic<bool> is_running{false};

void recv_func(RawSocket& socket) {
  std::array<char, 65536> recv_buf;

  while (is_running) {
    // Receive packet
    int read_bytes = socket.recv_from(recv_buf);
    if (read_bytes <= 0) {
      continue;
    }

    auto header = RawSocket::get_header(recv_buf);
    std::print("\nethernet [");
    std::print("src {:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}",
               header->h_source[0], header->h_source[1], header->h_source[2],
               header->h_source[3], header->h_source[4], header->h_source[5]);
    std::print(", dest {:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}",
               header->h_dest[0], header->h_dest[1], header->h_dest[2],
               header->h_dest[3], header->h_dest[4], header->h_dest[5]);
    std::println(", proto {}]", header->h_proto);

    auto payload = RawSocket::get_payload(recv_buf);
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
  constexpr std::array<uint8_t, 6> destination_mac = {0x00, 0x00, 0x00,
                                                      0x00, 0x00, 0x00};

  constexpr const char* DEFAULT_INTERFACE = "lo";

  const char* interface_name;

  // Get interface name
  if (argc > 1) {
    interface_name = argv[1];
  } else {
    interface_name = DEFAULT_INTERFACE;
  }

  RawSocket socket(interface_name);
  socket.bind(interface_name);
  std::println("Bound raw socket to \"{}\"", interface_name);

  std::array<char, 1024> send_buf;
  size_t tx_len = 0;

  // Packet data
  send_buf[tx_len++] = static_cast<char>(0xde);
  send_buf[tx_len++] = static_cast<char>(0xad);
  send_buf[tx_len++] = static_cast<char>(0xbe);
  send_buf[tx_len++] = static_cast<char>(0xef);

  is_running = true;
  std::thread recv_thread{recv_func, std::ref(socket)};

  while (1) {
    // Send packet
    if (socket.send_to(destination_mac, {send_buf.data(), tx_len}) < 0) {
      std::println("Send failed");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  is_running = false;
  recv_thread.join();
}
