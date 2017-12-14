#ifndef COMMON_TRANSPORT_STREAM_RECV_CLIENT_HPP
#define COMMON_TRANSPORT_STREAM_RECV_CLIENT_HPP

#include <functional>

#include "common/engine.hpp"
#include "common/transport/buffer.hpp"
#include "common/transport/endpoint.hpp"

namespace common {

namespace transport {

typedef std::function<size_t(buffer::const_iterator, buffer::const_iterator)>
    parser_t;

class stream_recv_client {
  endpoint m_ep;
  boost::asio::ip::tcp::socket m_socket;
  common::engine::timer_t m_retry_timer;
  buffer m_buffer;
  parser_t m_parser;

 public:
  stream_recv_client(common::engine::engine& engine,
                     endpoint const& ep,
                     parser_t const& parser);
  ~stream_recv_client();
  stream_recv_client(stream_recv_client const&) = delete;
  void operator=(stream_recv_client const&) = delete;

 private:
  void connection_handler(boost::system::error_code const& ec);
  void recv_handler(boost::system::error_code const& ec,
                    size_t bytes_transferred,
                    size_t bytes_offset);
  void start_retry_timer(std::chrono::seconds const& s);
  void retry_timer_handler(boost::system::error_code const& ec);
};

}  // namespace transport

}  // namespace common

#endif
