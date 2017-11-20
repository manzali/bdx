#include <iostream>

#include "common/log.hpp"

int main() {

  common::log::init();

  common::log::add_console();
  common::log::set_console_severity(common::log::severity_level::trace);

  common::log::add_file("logging", 1024 * 1024);
  common::log::set_file_severity(common::log::severity_level::info);

  LOG_INFO<< "test " << "with define";
  common::log::log(common::log::severity_level::info) << "test " << "w/o define";

  LOG_TRACE << "trace";// not printed to file
  LOG_DEBUG << "debug";// not printed to file
  LOG_INFO << "info";
  LOG_WARNING << "warning";
  LOG_ERROR << "error";
  LOG_FATAL << "fatal";

  return EXIT_SUCCESS;
}
