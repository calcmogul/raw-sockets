// Copyright (c) Tyler Veness. All Rights Reserved.

#include "raw_socket.hpp"

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <system_error>

RawSocket::RawSocket(std::string_view interface_name) {
  // Open RAW socket to send on
  m_sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (m_sockfd == -1) {
    throw std::system_error(errno, std::system_category(), "socket");
  }

  // Get the index of the interface to send on
  std::memset(&m_if_idx, 0, sizeof(struct ifreq));
  std::strncpy(m_if_idx.ifr_name, interface_name.data(), interface_name.size());
  if (ioctl(m_sockfd, SIOCGIFINDEX, &m_if_idx) < 0) {
    throw std::system_error(errno, std::system_category(), "SIOCGIFINDEX");
  }

  // Get the MAC address of the interface to send on
  std::memset(&m_if_mac, 0, sizeof(struct ifreq));
  std::strncpy(m_if_mac.ifr_name, interface_name.data(), interface_name.size());
  if (ioctl(m_sockfd, SIOCGIFHWADDR, &m_if_mac) < 0) {
    throw std::system_error(errno, std::system_category(), "SIOCGIFHWADDR");
  }

  // Set interface to promiscuous mode
  struct ifreq ifopts;
  std::strncpy(ifopts.ifr_name, interface_name.data(), interface_name.size());
  ioctl(m_sockfd, SIOCGIFFLAGS, &ifopts);
  ifopts.ifr_flags |= IFF_PROMISC;
  ioctl(m_sockfd, SIOCSIFFLAGS, &ifopts);

  // Allow socket to be reused if connection closes prematurely
  int sockopt;
  if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt,
                 sizeof(sockopt)) == -1) {
    throw std::system_error(errno, std::system_category(), "SO_REUSEADDR");
    close(m_sockfd);
  }
}

RawSocket::~RawSocket() { close(m_sockfd); }

void RawSocket::bind(std::string_view interface_name) {
  if (setsockopt(m_sockfd, SOL_SOCKET, SO_BINDTODEVICE, interface_name.data(),
                 interface_name.size()) < 0) {
    throw std::system_error(
        errno, std::system_category(),
        std::string{"RawSocket::bind(): "}.append(interface_name));
  }
}

ssize_t RawSocket::send_to(
    const std::array<uint8_t, MAC_OCTETS>& destination_mac,
    std::span<const char> buf) {
  auto eh = reinterpret_cast<struct ether_header*>(m_tx_buffer.data());

  // Ethernet header
  for (size_t i = 0; i < MAC_OCTETS; ++i) {
    eh->ether_shost[i] =
        reinterpret_cast<uint8_t*>(&m_if_mac.ifr_hwaddr.sa_data)[i];
    eh->ether_dhost[i] = destination_mac[i];
  }

  // Ethertype field
  eh->ether_type = htons(ETH_P_IP);

  // Packet data
  const size_t payload_length = std::min(buf.size(), MAX_DATAGRAM_SIZE);
  const size_t packet_length = sizeof(struct ether_header) + payload_length;
  std::memcpy(m_tx_buffer.data() + sizeof(struct ether_header), buf.data(),
              payload_length);

  struct sockaddr_ll socket_address;

  // Index of the network device
  socket_address.sll_ifindex = m_if_idx.ifr_ifindex;

  // Address length
  socket_address.sll_halen = ETH_ALEN;

  // Destination MAC
  for (size_t i = 0; i < MAC_OCTETS; ++i) {
    socket_address.sll_addr[i] = destination_mac[i];
  }

  // Send packet
  return sendto(m_sockfd, m_tx_buffer.data(), packet_length, 0,
                reinterpret_cast<struct sockaddr*>(&socket_address),
                sizeof(struct sockaddr_ll));
}

const struct ethhdr* RawSocket::get_header(std::span<const char> buf) {
  return reinterpret_cast<const struct ethhdr*>(buf.data());
}

std::span<const char> RawSocket::get_payload(std::span<const char> buf) {
  return {buf.data() + sizeof(struct ethhdr),
          buf.size() - sizeof(struct ethhdr)};
}
