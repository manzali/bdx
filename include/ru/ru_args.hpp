#ifndef RU_RU_ARGS_HPP
#define RU_RU_ARGS_HPP

#include <cstdlib> // exit
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace ru {

namespace po = boost::program_options;

struct ru_args {
  uint64_t id;
  boost::filesystem::path config_file;
};

ru_args parse_ru_args(int argc, char* argv[]) {

  po::options_description desc("options");
  ru_args args;

  desc.add_options()("help,h", "print help messages.")(
      "id,i",
      po::value<uint64_t>(&args.id)->required(),
      "readout unit id.")(
      "config,c",
      po::value<boost::filesystem::path>(&args.config_file)->required(),
      "configuration file.");

  try {

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      exit(EXIT_SUCCESS);
    }

    po::notify(vm);

  } catch (po::error const& e) {
    std::cerr << e.what() << '\n' << desc << std::endl;
    exit(EXIT_FAILURE);
  }

  return args;
}

}

#endif
