#ifndef COMMON_LOG_HPP
#define COMMON_LOG_HPP

#include <sstream>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

#define BOOST_LOG_DYN_LINK 1 // necessary when linking the boost_log library dynamically

// Define level macros
#define LEVEL_TRACE common::log::severity_level::trace
#define LEVEL_DEBUG common::log::severity_level::debug
#define LEVEL_INFO common::log::severity_level::info
#define LEVEL_WARNING common::log::severity_level::warning
#define LEVEL_ERROR common::log::severity_level::error
#define LEVEL_FATAL common::log::severity_level::fatal

// Define log macros
#define LOG_TRACE common::log::log(common::log::severity_level::trace)
#define LOG_DEBUG common::log::log(common::log::severity_level::debug)
#define LOG_INFO common::log::log(common::log::severity_level::info)
#define LOG_WARNING common::log::log(common::log::severity_level::warning)
#define LOG_ERROR common::log::log(common::log::severity_level::error)
#define LOG_FATAL common::log::log(common::log::severity_level::fatal)

namespace common {

namespace log {

typedef boost::log::trivial::severity_level severity_level;

void init();

void add_console();
void set_console_severity(severity_level const& level);
void remove_console();

void add_file(std::string const& file, size_t bytes);
void set_file_severity(severity_level const& level);
void remove_file();

class log {

 public:

  log(severity_level level);
  ~log();

  log(log const&) = delete;
  void operator=(log const&) = delete;

  template<typename T>
  log& operator <<(T const& t) {
    m_s << t;
    return *this;
  }

 private:

  std::stringstream m_s;
  severity_level m_level;

};

}

}

#endif
