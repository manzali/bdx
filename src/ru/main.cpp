#include "ru/ru_args.hpp"

#include "common/log.hpp"
#include "common/json.hpp"

int main(int argc, char* argv[]) {

  // parse command line options (it doesn't raise an exception)
  ru::ru_args const args = ru::parse_ru_args(argc, argv);

  try {

    // initialize the log component
    common::log::init();

    // add log to stdout console
    common::log::add_console();

    // parse configuration file and get json object
    common::json::object const config = common::json::read_json(
        args.config_file);

    // print json object
    // TODO: try to understand why "LOG_DEBUG<< config;" doesn't compile
    std::ostringstream oss;
    oss << config;
    LOG_DEBUG<< "configuration dump:\n" << oss.str();

  } catch(std::runtime_error& e) {

    LOG_FATAL<< e.what();
    return EXIT_FAILURE;

  }

  return EXIT_SUCCESS;
}
