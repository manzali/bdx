#ifndef RU_DATAFRAMES_MANAGER_HPP
#define RU_DATAFRAMES_MANAGER_HPP

#include <mutex>

#include "common/engine.hpp"
#include "common/transport/buffer.hpp"
#include "common/transport/endpoint.hpp"
#include "common/transport/exception.hpp"

namespace ru {

class dataframes_manager {
  common::engine::engine& m_engine;
  std::mutex m_mutex;
  std::map<uint64_t, std::vector<common::transport::buffer>> m_dataframes;

 public:
  dataframes_manager(common::engine::engine& engine);
  ~dataframes_manager();
  dataframes_manager(dataframes_manager const&) = delete;
  void operator=(dataframes_manager const&) = delete;

  size_t dataframes_parser(common::transport::buffer::const_iterator b,
                           common::transport::buffer::const_iterator e);

 private:
  void timeslices_parser(
      std::vector<common::transport::buffer> const& dataframes);
};

}  // namespace ru

#endif
