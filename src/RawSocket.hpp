// Copyright (c) 2018-2020 Tyler Veness. All Rights Reserved.

#pragma once

#include <net/if.h>
#include <stdint.h>

#include <array>
#include <string_view>

class RawSocket {
 public:
  // Number of octets in MAC address
  static constexpr size_t kMacOctets = 6;

  explicit RawSocket(std::string_view interfaceName);
  ~RawSocket();

  /**
   * Bind the socket to the specified interface.
   *
   * Throws a std::system_error exception if the socket failed to bind.
   */
  void Bind(std::string_view interfaceName);

  /**
   * Sends the data referenced by the std::string_view to the destination MAC
   * address.
   *
   * @param destinationMac The MAC address of the destination interface.
   * @param buf            The buffer containing the payload.
   * @return The number of bytes sent or -1 on error.
   */
  ssize_t SendTo(const std::array<uint8_t, kMacOctets>& destinationMac,
                 std::string_view buf);

  /**
   * Reads up to the number of bytes the std::string_view buf can hold and
   * stores them there.
   *
   * @param buf The buffer in which to store read bytes.
   * @return Number of bytes received or -1 on error.
   */
  template <size_t N>
  ssize_t RecvFrom(std::array<char, N>& buf) {
    // Receive packet
    return recv(m_sockfd, reinterpret_cast<char*>(buf.data()), buf.size(), 0);
  }

  /**
   * Prints out ethernet header data from a packet received via RecvFrom().
   *
   * @param buf The buffer containing the ethernet frame.
   */
  static void ParseHeader(std::string_view buf);

  /**
   * Returns pointer to payload in a packet received via RecvFrom().
   *
   * @param buf The buffer containing the ethernet frame.
   * @param len Size of ethernet frame.
   * @return A std::string_view of the payload.
   */
  static std::string_view GetPayload(std::string_view buf);

 private:
  static constexpr size_t kMaxDatagramSize = 65507;

  struct ifreq m_if_idx;
  struct ifreq m_if_mac;
  int m_sockfd = -1;
  std::array<uint8_t, kMaxDatagramSize> m_txBuffer;
};
