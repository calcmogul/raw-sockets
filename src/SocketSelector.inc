// Copyright (c) 2018 Tyler Veness. All Rights Reserved.

#pragma once

template <typename Rep, typename Period>
int SocketSelector::Select(const std::chrono::duration<Rep, Period>& timeout) {
  m_readfds = m_in_readfds;
  m_writefds = m_in_writefds;
  m_exceptfds = m_in_exceptfds;

  struct timeval val;
  val.tv_sec =
      std::chrono::duration_cast<std::chrono::seconds>(timeout).count();
  val.tv_sec =
      std::chrono::duration_cast<std::chrono::microseconds>(timeout).count() %
      1000000;

  return select(m_maxSocket + 1, &m_readfds, &m_writefds, &m_exceptfds, &val);
}
