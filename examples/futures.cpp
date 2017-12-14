#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/use_future.hpp>
#include "common/engine.hpp"

int main()
{
  common::engine::engine engine(1);

  boost::asio::ip::tcp::resolver resolver(engine.get());

  boost::asio::ip::tcp::resolver::query query(
      boost::asio::ip::tcp::v4(), "127.0.0.1", "5600");

  auto f_iter = resolver.async_resolve(query, boost::asio::use_future);

  boost::asio::ip::tcp::resolver::iterator iter;
  try {
    iter = f_iter.get();
  } catch (const boost::system::system_error& e) {
    std::cerr << "async_resolve: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  boost::asio::ip::tcp::socket socket(engine.get());

  auto f_connection =
      boost::asio::async_connect(socket, iter, boost::asio::use_future);

  try {
    iter = f_connection.get();
  } catch (const boost::system::system_error& e) {
    std::cerr << "async_connect: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  size_t const max_buffer_size = 1024 * 1024;
  size_t const min_transfered_bytes = 1;

  std::vector<unsigned char> buffer(max_buffer_size);

  auto f_len = boost::asio::async_read(
      socket,
      boost::asio::buffer(&buffer.front(), buffer.size()),
      boost::asio::transfer_at_least(min_transfered_bytes),
      boost::asio::use_future);

  size_t len;
  try {
    len = f_len.get();
  } catch (const boost::system::system_error& e) {
    std::cerr << "async_read: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "read " << len << " bytes" << std::endl;

  engine.stop();

  return EXIT_SUCCESS;
}
