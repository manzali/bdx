#ifndef COMMON_TRANSPORT_ENDPOINT_HPP
#define COMMON_TRANSPORT_ENDPOINT_HPP

#include <boost/asio.hpp>

namespace common {

namespace transport {

typedef boost::asio::ip::tcp::endpoint endpoint;

inline endpoint make_endpoint(std::string const& host, std::string const& port)
{
  boost::asio::io_service service;
  boost::asio::ip::tcp::resolver resolver(service);
  boost::asio::ip::tcp::resolver::query query(
      boost::asio::ip::tcp::v4(), host, port);
  return *resolver.resolve(query);
}

}  // namespace transport

}  // namespace common

#endif
