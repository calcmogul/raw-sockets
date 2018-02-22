// Copyright (c) 2018 Tyler Veness. All Rights Reserved.

#include "RawSocket.hpp"

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <system_error>

#include "SocketSelector.hpp"

RawSocket::RawSocket(const std::string& interfaceName) {
  // Open RAW socket to send on
  m_sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
  if (m_sockfd == -1) {
    throw std::system_error(errno, std::system_category(), "socket");
  }

  // Get the index of the interface to send on
  std::memset(&m_if_idx, 0, sizeof(struct ifreq));
  std::strncpy(m_if_idx.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);
  if (ioctl(m_sockfd, SIOCGIFINDEX, &m_if_idx) < 0) {
    throw std::system_error(errno, std::system_category(), "SIOCGIFINDEX");
  }

  // Get the MAC address of the interface to send on
  std::memset(&m_if_mac, 0, sizeof(struct ifreq));
  std::strncpy(m_if_mac.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);
  if (ioctl(m_sockfd, SIOCGIFHWADDR, &m_if_mac) < 0) {
    throw std::system_error(errno, std::system_category(), "SIOCGIFHWADDR");
  }
}

RawSocket::~RawSocket() { close(m_sockfd); }

int RawSocket::SendTo(const std::array<uint8_t, kMaxDestMacs>& destinationMacs,
                      const void* buf, size_t len) {
  auto eh = reinterpret_cast<struct ether_header*>(m_txBuffer.data());

  // Ethernet header
  for (size_t i = 0; i < kMaxDestMacs; i++) {
    eh->ether_shost[i] =
        reinterpret_cast<uint8_t*>(&m_if_mac.ifr_hwaddr.sa_data)[i];
    eh->ether_dhost[i] = destinationMacs[i];
  }

  // Ethertype field
  eh->ether_type = htons(ETH_P_IP);

  // Packet data
  const size_t payloadLength = std::min(len, kMaxDatagramSize);
  const size_t packetLength = sizeof(struct ether_header) + payloadLength;
  std::memcpy(m_txBuffer.data() + sizeof(struct ether_header),
              static_cast<const char*>(buf), payloadLength);

  struct sockaddr_ll socket_address;

  // Index of the network device
  socket_address.sll_ifindex = m_if_idx.ifr_ifindex;

  // Address length
  socket_address.sll_halen = ETH_ALEN;

  // Destination MAC
  for (size_t i = 0; i < kMaxDestMacs; ++i) {
    socket_address.sll_addr[i] = destinationMacs[i];
  }

  // Send packet
  return sendto(m_sockfd, m_txBuffer.data(), packetLength, 0,
                reinterpret_cast<struct sockaddr*>(&socket_address),
                sizeof(struct sockaddr_ll));
}

int RawSocket::Recv(void* buf, size_t len) {
  int ret;
  size_t numRead = 0;

  SocketSelector selector;

  while (numRead < len) {
    selector.Zero(SocketSelector::read | SocketSelector::except);

    // Set the sockets into the fd_set s
    selector.AddSocket(m_sockfd, SocketSelector::read | SocketSelector::except);

    ret = selector.Select();

    if (ret == -1) {
      return -1;
    }

    // If an exception occurred with the socket, return error.
    if (selector.IsReady(m_sockfd, SocketSelector::except)) {
      return -1;
    }

    // Otherwise, read some more.
    ret = recv(m_sockfd, static_cast<char*>(buf) + numRead, len - numRead, 0);
    if (ret < 1) {
      return -1;
    }
    numRead += ret;
  }

  return numRead;
}
