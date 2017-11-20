#include <iostream>

#include "common/engine.hpp"
#include "common/transport/stream_recv_client.hpp"

std::vector<size_t> group_256_bytes(
    std::vector<uint8_t> const& buffer,
    size_t len) {
  size_t const group_by = 256;
  std::vector<size_t> messages;
  std::vector<uint8_t>::const_iterator it = std::begin(buffer);
  std::vector<uint8_t>::const_iterator end = std::begin(buffer) + len;
  it += group_by;
  while (it <= end) {
    messages.push_back(group_by);
    it += group_by;
  }
  return messages;
}

std::vector<size_t> default_parser(
    std::vector<uint8_t> const& buffer,
    size_t len) {
  return std::vector<size_t> {len};
}

int main() {

  common::engine::engine engine(1);

  auto const ep = common::transport::make_endpoint("127.0.0.1", "5600");
  common::transport::stream_recv_client receiver(
      engine,
      ep,
      1024 * 1024,
      default_parser);
//  group_256_bytes);

  std::this_thread::sleep_for(std::chrono::seconds(300));

  engine.stop();

  return EXIT_SUCCESS;
}
