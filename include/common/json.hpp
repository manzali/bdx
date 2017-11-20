#ifndef COMMON_JSON_HPP
#define COMMON_JSON_HPP

#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace common {

namespace json {

typedef boost::property_tree::ptree object;

inline object read_json(std::istream& is) {
  object obj;
  try {
    read_json(is, obj);
  } catch (boost::property_tree::json_parser_error const& e) {
    throw std::runtime_error(
        "error parsing the configuration file in json: "
            + std::string(e.what()));
  }
  return obj;
}

inline object read_json(boost::filesystem::path const& file) {
  if (!boost::filesystem::exists(file)) {
    throw std::runtime_error("file " + file.string() + " does not exist");
    exit(EXIT_FAILURE);
  } else if (!boost::filesystem::is_regular_file(file)) {
    throw std::runtime_error(file.string() + " is not a regular file");
  }
  std::ifstream ifs(file.string());
  if (!ifs) {
    throw std::runtime_error("cannot open file " + file.string());
  }
  return read_json(ifs);
}

}

}

inline std::ostream& operator<<(
    std::ostream& os,
    common::json::object const& obj) {
  write_json(os, obj);
  return os;
}

#endif
