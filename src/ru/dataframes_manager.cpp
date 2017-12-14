#include "ru/dataframes_manager.hpp"

#include "common/dataformat/f_dataformat.hpp"
#include "common/dataformat/s_dataformat.hpp"
#include "common/log.hpp"

namespace ru {

using namespace common::dataformat;
using namespace common::transport;

dataframes_manager::dataframes_manager(common::engine::engine& engine)
    : m_engine(engine)
{
}

dataframes_manager::~dataframes_manager()
{
}

size_t dataframes_manager::dataframes_parser(buffer::const_iterator b,
                                             buffer::const_iterator e)
{
  buffer::const_iterator it = b;
  bool searching = true;

  std::map<uint64_t, std::vector<common::transport::buffer>> dataframes;

  while (searching) {
    if (it + sizeof(DataFrameHeader) < e) {
      DataFrameHeader const& dfh = *dataframeheader_cast(&(*it));
      // check syncbyte
      if (!testDFHSync(dfh)) {
        throw common::transport::parser_error("found wrong syncbyte");
      }
      uint64_t const df_id = dfh.PMTID;
      // TODO: do you want any other check?
      size_t const size_of_payload = getDFHPayloadSize(dfh);
      auto const df_end = it + sizeof(DataFrameHeader) + size_of_payload;
      if (df_end < e) {
        if (!subsequent(dfh)) {  // WARNING: now you are ignoring all subsequent
                                 // dataframes
          dataframes[df_id].emplace_back(it, df_end);
        }
        it = df_end;
      } else {
        searching = false;
      }
    } else {
      searching = false;
    }
  }
  for (auto& pair : dataframes) {
    if (!pair.second.empty()) {
      m_engine.post(std::bind(
          &dataframes_manager::timeslices_parser, this, std::move(pair.second)));
    }
  }
  return std::distance(b, it);
}

void dataframes_manager::timeslices_parser(
    std::vector<common::transport::buffer> const&
        dataframes)  // CHECK: does it work correctly with const& ?
{
  // get id
  DataFrameHeader const& dfh = *dataframeheader_cast(&std::begin(dataframes)->front());
  uint64_t const df_id = dfh.PMTID;

  // acquire the lock
  std::unique_lock<std::mutex> mlock(m_mutex);

  // get the map value
  auto& xxx = m_dataframes[df_id];

  // append new dataframes
  xxx.insert(
      std::end(xxx), std::begin(dataframes), std::end(dataframes));
  auto it = std::begin(xxx);
  while (it != std::end(xxx)) {
    // get current timeslice id
    uint64_t const ts_id = getTimesliceId(*dataframeheader_cast(&it->front()),
                                          boost::chrono::milliseconds(200));
    // search for timeslice completion
    it = std::find_if(
        std::begin(xxx),
        std::end(xxx),
        [&ts_id](common::transport::buffer const& buffer) {
          return ts_id != getTimesliceId(*dataframeheader_cast(&buffer.front()),
                                         boost::chrono::milliseconds(200));
        });
    if (it != std::end(xxx)) {
      LOG_DEBUG << "Completed timeslice " << ts_id << " for id " << df_id << " with "
                << std::distance(std::begin(xxx), it) << " dataframes";

      // do what you want with the completed ts
      // ........

      // remove the completed ts
      it = xxx.erase(std::begin(xxx), it);
    }
  }
}

}  // namespace ru
