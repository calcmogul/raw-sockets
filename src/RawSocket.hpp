// Copyright (c) 2018 Tyler Veness. All Rights Reserved.

#pragma once

#include <net/if.h>
#include <stdint.h>

#include <array>
#include <string>

class RawSocket {
 public:
  // Maximum number of destination MAC addresses that can be sent to in one
  // sendto(2) call
  static constexpr size_t kMaxDestMacs = 6;

  explicit RawSocket(const std::string& interfaceName);
  ~RawSocket();

  int SendTo(const std::array<uint8_t, kMaxDestMacs>& destinationMacs,
             const void* buf, size_t len);

  /**
   * Reads up to len bytes from the socket and stores them in buf.
   *
   * @param buf The buffer in which to store read bytes.
   * @param len The maximum number of bytes to read from the socket.
   * @return The number of bytes actually read or -1 on error.
   */
  int Recv(void* buf, size_t len);

 private:
  static constexpr size_t kMaxDatagramSize = 65507;

  struct ifreq m_if_idx;
  struct ifreq m_if_mac;
  int m_sockfd = -1;
  std::array<uint8_t, kMaxDatagramSize> m_txBuffer;
};
