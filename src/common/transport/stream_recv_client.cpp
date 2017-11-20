#include <iostream>

#include <boost/bind.hpp>

#include "common/transport/stream_recv_client.hpp"
#include "common/make_unique.hpp"

namespace common {

namespace transport {

namespace {

size_t const min_transfered_bytes = 1;
long const retry_time = 1;

/*
 std::vector<size_t> default_parser(
 buffer const& buf,
 size_t len) {
 return std::vector<size_t> { len };
 }
 */
}

stream_recv_client::stream_recv_client(
    common::engine::engine& engine,
    endpoint const& ep,
    size_t max_buffer_size,
    std::function<std::vector<size_t>(buffer const&, size_t)> const& parser)
    : m_ep(ep),
      m_socket(engine.get()),
      m_retry_timer(engine.get()),
      m_buffer(common::pointer::make_unique<buffer>(max_buffer_size)),
      m_parser(parser) {
  retry_timer_handler(boost::system::error_code());
}

stream_recv_client::~stream_recv_client() {
  boost::system::error_code ec;
  m_retry_timer.cancel(ec);
  if (m_socket.is_open()) {
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    m_socket.close(ec);
  }
}

void stream_recv_client::connection_handler(
    boost::system::error_code const& ec) {

  if (!ec) {

    std::cout << "Connection to " << m_ep << " succeeded" << std::endl;

    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(&m_buffer->front(), m_buffer->size()),
        boost::asio::transfer_at_least(min_transfered_bytes),
        boost::bind(
            &stream_recv_client::recv_handler,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            0));

  } else {

    std::cout << "Error connecting to "
              << m_ep
              << ": "
              << ec.message()
              << std::endl;

    start_retry_timer(retry_time);
  }

}

void stream_recv_client::recv_handler(
    boost::system::error_code const& ec,
    std::size_t bytes_transferred,
    size_t bytes_offset) {

  if (!ec) {

    // update bytes_transferred
    bytes_transferred += bytes_offset;

    auto messages = m_parser(*m_buffer, bytes_transferred);

    size_t bytes_parsed = 0;

    for (auto message : messages) {

      // check if the size is not zero

      // check if bytes_parsed > buffer size

      std::unique_ptr<buffer> temp = common::pointer::make_unique<buffer>(
          std::begin(*m_buffer) + bytes_parsed,
          std::begin(*m_buffer) + bytes_parsed + message);

      if (temp) {
        std::cout << "Received "
                  << temp->size()
                  << " bytes from "
                  << m_ep
                  << std::endl;
      }

      bytes_parsed += message;
    }

    // update bytes_offset
    assert(bytes_transferred >= bytes_parsed);
    bytes_offset = bytes_transferred - bytes_parsed;

    // move the remaining data to the beginning of the buffer
    std::memmove(
        &m_buffer->front(),
        &m_buffer->front() + bytes_parsed,
        bytes_offset);

    // recall the read
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(
            &m_buffer->front() + bytes_offset,
            m_buffer->size() - bytes_offset),
        boost::asio::transfer_at_least(min_transfered_bytes),
        boost::bind(
            &stream_recv_client::recv_handler,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            bytes_offset));

  } else {
    std::cout << "Error receiving from "
              << m_ep
              << ": "
              << ec.message()
              << std::endl;

    start_retry_timer(retry_time);
  }
}

void stream_recv_client::start_retry_timer(long seconds) {
  m_retry_timer.expires_from_now(boost::posix_time::seconds(seconds));
  m_retry_timer.async_wait(
      boost::bind(
          &stream_recv_client::retry_timer_handler,
          this,
          boost::asio::placeholders::error));
}

void stream_recv_client::retry_timer_handler(
    boost::system::error_code const& ec) {

  if (ec != boost::asio::error::operation_aborted) {
    m_socket.close();
    m_socket.async_connect(
        m_ep,
        boost::bind(
            &stream_recv_client::connection_handler,
            this,
            boost::asio::placeholders::error));
  }

}

}

}
