#ifndef COMMON_TRANSPORT_EXCEPTION_HPP
#define COMMON_TRANSPORT_EXCEPTION_HPP

#include <stdexcept>

namespace common {

namespace transport {

class parser_error : public std::runtime_error {
 public:
  explicit parser_error(std::string const& error)
      : std::runtime_error(error) {
  }
};

}

}

#endif
