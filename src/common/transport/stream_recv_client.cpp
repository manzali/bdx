#include "common/transport/stream_recv_client.hpp"

#include <algorithm>

#include <boost/bind.hpp>

#include "common/log.hpp"
#include "common/transport/exception.hpp"

namespace common {

namespace transport {

namespace {

size_t const max_buffer_size = 1024 * 1024;
size_t const min_transfered_bytes = 1;
std::chrono::seconds const connect_retry_time(1);

}  // namespace

stream_recv_client::stream_recv_client(common::engine::engine& engine,
                                       endpoint const& ep,
                                       parser_t const& parser)
    : m_ep(ep)
    , m_socket(engine.get())
    , m_retry_timer(engine.get())
    , m_buffer(max_buffer_size)
    , m_parser(parser)
{
  retry_timer_handler(boost::system::error_code());
}

stream_recv_client::~stream_recv_client()
{
  boost::system::error_code ec;
  m_retry_timer.cancel(ec);
  if (m_socket.is_open()) {
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    m_socket.close(ec);
  }
}

void stream_recv_client::connection_handler(boost::system::error_code const& ec)
{
  if (!ec) {
    LOG_DEBUG << "Connection to " << m_ep << " succeeded";

    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(&m_buffer.front(), m_buffer.size()),
        boost::asio::transfer_at_least(min_transfered_bytes),
        boost::bind(&stream_recv_client::recv_handler,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred,
                    0));

  } else {
    LOG_ERROR << "Error connecting to " << m_ep << ": " << ec.message();
    start_retry_timer(connect_retry_time);
  }
}

void stream_recv_client::recv_handler(boost::system::error_code const& ec,
                                      size_t bytes_transferred,
                                      size_t bytes_offset)
{
  if (!ec) {
    assert(bytes_offset + bytes_transferred <= m_buffer.size());

    buffer::const_iterator it = std::begin(m_buffer) + bytes_offset;
    assert(it < std::end(m_buffer));

    buffer::const_iterator const end = it + bytes_transferred;
    assert(end <= std::end(m_buffer));

    // start parsing
    size_t bytes_parsed = 0;
    do {
      try {
        bytes_parsed = m_parser(it, end);
      } catch (common::transport::parser_error& e) {
        LOG_ERROR << "Error parsing data: " << e.what();
        start_retry_timer(connect_retry_time);
        return;
      }
      if (bytes_parsed) {
        it += bytes_parsed;
      }
    } while (it != end && bytes_parsed);

    // update bytes_offset
    bytes_offset = std::distance(it, end);

    // move the remaining data to the beginning of the buffer
    std::copy(it, end, std::begin(m_buffer));

    // recall the read
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(&m_buffer.front() + bytes_offset,
                            m_buffer.size() - bytes_offset),
        boost::asio::transfer_at_least(min_transfered_bytes),
        boost::bind(&stream_recv_client::recv_handler,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred,
                    bytes_offset));

  } else {
    LOG_ERROR << "Error receiving from " << m_ep << ": " << ec.message();
    start_retry_timer(connect_retry_time);
  }
}

void stream_recv_client::start_retry_timer(
    std::chrono::seconds const& connect_retry_time)
{
  m_retry_timer.expires_from_now(connect_retry_time);
  m_retry_timer.async_wait(boost::bind(&stream_recv_client::retry_timer_handler,
                                       this,
                                       boost::asio::placeholders::error));
}

void stream_recv_client::retry_timer_handler(
    boost::system::error_code const& ec)
{
  if (ec != boost::asio::error::operation_aborted) {
    m_socket.close();
    m_socket.async_connect(m_ep,
                           boost::bind(&stream_recv_client::connection_handler,
                                       this,
                                       boost::asio::placeholders::error));
  }
}

}  // namespace transport

}  // namespace common
