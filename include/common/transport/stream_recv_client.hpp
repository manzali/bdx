#ifndef COMMON_TRANSPORT_STREAM_RECV_CLIENT_HPP
#define COMMON_TRANSPORT_STREAM_RECV_CLIENT_HPP

#include <functional>

#include "common/engine.hpp"
#include "common/transport/endpoint.hpp"
#include "common/transport/buffer.hpp"

namespace common {

namespace transport {

class stream_recv_client {

  endpoint m_ep;
  boost::asio::ip::tcp::socket m_socket;
  boost::asio::deadline_timer m_retry_timer;
  std::unique_ptr<buffer> m_buffer;
  std::function<std::vector<size_t>(buffer const&, size_t)> m_parser;

 public:

  stream_recv_client(
      common::engine::engine& engine,
      endpoint const& ep,
      size_t max_buffer_size,
      std::function<std::vector<size_t>(buffer const&, size_t)> const& parser);
  ~stream_recv_client();
  stream_recv_client(stream_recv_client const&) = delete;
  void operator=(stream_recv_client const&) = delete;

 private:

  void connection_handler(boost::system::error_code const& ec);
  void recv_handler(
      boost::system::error_code const& ec,
      std::size_t bytes_transferred,
      size_t bytes_offset);
  void start_retry_timer(long seconds);
  void retry_timer_handler(boost::system::error_code const& ec);
};

}

}

#endif
