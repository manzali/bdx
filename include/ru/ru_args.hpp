#ifndef RU_RU_ARGS_HPP
#define RU_RU_ARGS_HPP

#include <chrono>
#include <cstdlib> // exit
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace ru {

namespace po = boost::program_options;

struct ru_args {
  std::string id;
  boost::filesystem::path config_file;
  std::chrono::seconds timeout;
};

ru_args parse_ru_args(int argc, char *argv[]) {

  po::options_description desc("options");
  ru_args args;

  int raw_timeout = 0;

  desc.add_options()("help,h", "print help messages.")(
      "id,i", po::value<std::string>(&args.id)->required(), "readout unit identification string.")(
      "config,c",
      po::value<boost::filesystem::path>(&args.config_file)->required(),
      "configuration file.")("timeout,t", po::value<int>(&raw_timeout),
                             "timeout in seconds (default = 0).");

  try {

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      exit(EXIT_SUCCESS);
    }

    po::notify(vm);

    // check timeout value
    if (raw_timeout < 0) {
      std::cerr << "wrong timeout value: " << raw_timeout << std::endl;
      exit(EXIT_FAILURE);
    }

    // convert timeout in chrono
    args.timeout = std::chrono::seconds(raw_timeout);

  } catch (po::error const &e) {
    std::cerr << e.what() << '\n' << desc << std::endl;
    exit(EXIT_FAILURE);
  }

  return args;
}

} // namespace ru

#endif
