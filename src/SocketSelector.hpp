// Copyright (c) 2018 Tyler Veness. All Rights Reserved.

#pragma once

#include <stdint.h>
#include <sys/select.h>

#include <chrono>

/**
 * A wrapper around select(3)
 */
class SocketSelector {
 public:
  SocketSelector();

  enum select_type { read = 1 << 0, write = 1 << 1, except = 1 << 2 };

  void AddSocket(int sd, uint32_t types);
  void RemoveSocket(int sd, uint32_t types);

  // Returns true if socket is going to be checked
  bool IsSelected(int sd, select_type type);

  // Returns true if socket is pending an action
  bool IsReady(int sd, select_type type);

  void Zero(uint32_t types);

  /**
   * Returns the number of sockets ready; timeout of 0 causes blocking on
   * select(3).
   */
  template <typename Rep, typename Period>
  int Select(const std::chrono::duration<Rep, Period>& timeout);

  /**
   * Returns the number of sockets ready, blocking on select(3).
   */
  int Select();

 private:
  fd_set m_readfds;    // set containing sockets to be checked for being ready
                       // to read
  fd_set m_writefds;   // set containing sockets to be checked for being ready
                       // to write
  fd_set m_exceptfds;  // set containing sockets to be checked for error
                       // conditions pending
  int m_maxSocket;     // maximum socket handle

  fd_set m_in_readfds;    // set containing sockets to be checked for being
                          // ready to read
  fd_set m_in_writefds;   // set containing sockets to be checked for being
                          // ready to write
  fd_set m_in_exceptfds;  // set containing sockets to be checked for error
                          // conditions pending
};

#include "SocketSelector.inc"
