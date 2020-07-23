// Copyright (c) 2018-2020 Tyler Veness. All Rights Reserved.

#include "RawSocket.hpp"

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <system_error>

RawSocket::RawSocket(std::string_view interfaceName) {
  // Open RAW socket to send on
  m_sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (m_sockfd == -1) {
    throw std::system_error(errno, std::system_category(), "socket");
  }

  // Get the index of the interface to send on
  std::memset(&m_if_idx, 0, sizeof(struct ifreq));
  std::strncpy(m_if_idx.ifr_name, interfaceName.data(), interfaceName.length());
  if (ioctl(m_sockfd, SIOCGIFINDEX, &m_if_idx) < 0) {
    throw std::system_error(errno, std::system_category(), "SIOCGIFINDEX");
  }

  // Get the MAC address of the interface to send on
  std::memset(&m_if_mac, 0, sizeof(struct ifreq));
  std::strncpy(m_if_mac.ifr_name, interfaceName.data(), interfaceName.length());
  if (ioctl(m_sockfd, SIOCGIFHWADDR, &m_if_mac) < 0) {
    throw std::system_error(errno, std::system_category(), "SIOCGIFHWADDR");
  }

  // Set interface to promiscuous mode
  struct ifreq ifopts;
  std::strncpy(ifopts.ifr_name, interfaceName.data(), interfaceName.length());
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

void RawSocket::Bind(std::string_view interfaceName) {
  if (setsockopt(m_sockfd, SOL_SOCKET, SO_BINDTODEVICE, interfaceName.data(),
                 interfaceName.length()) < 0) {
    throw std::system_error(
        errno, std::system_category(),
        std::string{"RawSocket::Bind(): "}.append(interfaceName));
  }
}

ssize_t RawSocket::SendTo(const std::array<uint8_t, kMacOctets>& destinationMac,
                          std::string_view buf) {
  auto eh = reinterpret_cast<struct ether_header*>(m_txBuffer.data());

  // Ethernet header
  for (size_t i = 0; i < kMacOctets; i++) {
    eh->ether_shost[i] =
        reinterpret_cast<uint8_t*>(&m_if_mac.ifr_hwaddr.sa_data)[i];
    eh->ether_dhost[i] = destinationMac[i];
  }

  // Ethertype field
  eh->ether_type = htons(ETH_P_IP);

  // Packet data
  const size_t payloadLength = std::min(buf.length(), kMaxDatagramSize);
  const size_t packetLength = sizeof(struct ether_header) + payloadLength;
  std::memcpy(m_txBuffer.data() + sizeof(struct ether_header), buf.data(),
              payloadLength);

  struct sockaddr_ll socket_address;

  // Index of the network device
  socket_address.sll_ifindex = m_if_idx.ifr_ifindex;

  // Address length
  socket_address.sll_halen = ETH_ALEN;

  // Destination MAC
  for (size_t i = 0; i < kMacOctets; i++) {
    socket_address.sll_addr[i] = destinationMac[i];
  }

  // Send packet
  return sendto(m_sockfd, m_txBuffer.data(), packetLength, 0,
                reinterpret_cast<struct sockaddr*>(&socket_address),
                sizeof(struct sockaddr_ll));
}

void RawSocket::ParseHeader(std::string_view buf) {
  auto eth = reinterpret_cast<const struct ethhdr*>(buf.data());

  std::printf("\nEthernet Header\n");
  std::printf("\t|-Source Address: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",
              eth->h_source[0], eth->h_source[1], eth->h_source[2],
              eth->h_source[3], eth->h_source[4], eth->h_source[5]);
  std::printf("\t|-Destination Address: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",
              eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3],
              eth->h_dest[4], eth->h_dest[5]);
  std::printf("\t|-Protocol: %d\n", eth->h_proto);
}

std::string_view RawSocket::GetPayload(std::string_view buf) {
  return {buf.data() + sizeof(struct ethhdr),
          buf.length() - sizeof(struct ethhdr)};
}
