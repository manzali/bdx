#include <sstream>

#include <boost/detail/lightweight_test.hpp>

#include "common/json.hpp"

int main() {

  std::stringstream ss;
  ss << "{\
        \"key1\": {\
          \"integer\": 123,\
          \"double\": 4.56,\
          \"bool\": true\
        },\
        \"key2\": [\
          {\
            \"array\": \"value\"\
          },\
          {\
            \"array\": \"value\"\
          },\
          {\
            \"array\": \"value\"\
          }\
        ],\
        \"key3\": \"abc\"\
      }";

  common::json::object const configuration = common::json::read_json(ss);

  // key1
  BOOST_TEST_EQ(configuration.get<int>("key1.integer"), 123);
  BOOST_TEST_EQ(configuration.get<double>("key1.double"), 4.56);
  BOOST_TEST_EQ(configuration.get<bool>("key1.bool"), true);

  // key2
  common::json::object const& child = configuration.get_child("key2");
  for (auto& item : child) {
    BOOST_TEST_EQ(item.second.get<std::string>("array"), "value");
  }

  // key3
  BOOST_TEST_EQ(configuration.get<std::string>("key3"), "abc");

  // missing key
  BOOST_TEST_THROWS(
      configuration.get<std::string>("key4"),
      boost::property_tree::ptree_error);

  return boost::report_errors();
}
