// Copyright (c) Tyler Veness

#pragma once

#include <net/if.h>
#include <netinet/ether.h>
#include <stdint.h>

#include <array>
#include <span>
#include <string_view>

class RawSocket {
 public:
  // Number of octets in MAC address
  static constexpr size_t MAC_OCTETS = 6;

  explicit RawSocket(std::string_view interface_name);
  ~RawSocket();

  /**
   * Bind the socket to the specified interface.
   *
   * Throws a std::system_error exception if the socket failed to bind.
   */
  void bind(std::string_view interface_name);

  /**
   * Sends the data referenced by the span to the destination MAC address.
   *
   * @param destination_mac The MAC address of the destination interface.
   * @param buf             The buffer containing the payload.
   * @return The number of bytes sent or -1 on error.
   */
  ssize_t send_to(const std::array<uint8_t, MAC_OCTETS>& destination_mac,
                  std::span<const char> buf);

  /**
   * Reads up to the number of bytes the span buf can hold and stores them
   * there.
   *
   * @param buf The buffer in which to store read bytes.
   * @return Number of bytes received or -1 on error.
   */
  template <size_t N>
  ssize_t recv_from(std::array<char, N>& buf) {
    // Receive packet
    return recv(m_sockfd, reinterpret_cast<char*>(buf.data()), buf.size(), 0);
  }

  /**
   * Returns header from a packet received via RecvFrom().
   *
   * @param buf The buffer containing the ethernet frame.
   */
  static const struct ethhdr* get_header(std::span<const char> buf);

  /**
   * Returns span of payload from a packet received via RecvFrom().
   *
   * @param buf The buffer containing the ethernet frame.
   * @param len Size of ethernet frame.
   * @return A span of the payload.
   */
  static std::span<const char> get_payload(std::span<const char> buf);

 private:
  static constexpr size_t MAX_DATAGRAM_SIZE = 65507;

  struct ifreq m_if_idx;
  struct ifreq m_if_mac;
  int m_sockfd = -1;
  std::array<uint8_t, MAX_DATAGRAM_SIZE> m_tx_buffer;
};
