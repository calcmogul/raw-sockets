// Copyright (c) 2018 Tyler Veness. All Rights Reserved.

#pragma once

#include <net/if.h>
#include <stdint.h>

#include <array>
#include <string>

#include "StringView.hpp"

class RawSocket {
 public:
  // Number of octets in MAC address
  static constexpr size_t kMacOctets = 6;

  explicit RawSocket(const std::string& interfaceName);
  ~RawSocket();

  /**
   * Bind the socket to the specified interface.
   *
   * Throws a std::system_error exception if the socket failed to bind.
   */
  void Bind(std::string interfaceName);

  /**
   * Sends the data referenced by the StringView to the destination MAC address.
   *
   * @param destinationMac The MAC address of the destination interface.
   * @param buf            The buffer containing the payload.
   * @return The number of bytes sent or -1 on error.
   */
  int SendTo(const std::array<uint8_t, kMacOctets>& destinationMac,
             const StringView buf);

  /**
   * Reads up to the number of bytes the StringView buf can hold and stores them
   * there.
   *
   * @param buf The buffer in which to store read bytes.
   * @return A StringView of buf or a null StringView on error.
   */
  StringView RecvFrom(StringView buf);

  /**
   * Prints out ethernet header data from a packet received via RecvFrom().
   *
   * @param buf The buffer containing the ethernet frame.
   */
  static void ParseHeader(StringView buf);

  /**
   * Returns pointer to payload in a packet received via RecvFrom().
   *
   * @param buf The buffer containing the ethernet frame.
   * @param len Size of ethernet frame.
   * @return A StringView of the payload.
   */
  static StringView GetPayload(StringView buf);

 private:
  static constexpr size_t kMaxDatagramSize = 65507;

  struct ifreq m_if_idx;
  struct ifreq m_if_mac;
  int m_sockfd = -1;
  std::array<uint8_t, kMaxDatagramSize> m_txBuffer;
};
