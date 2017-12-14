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
  std::vector<common::transport::buffer> dataframes;
  while (searching) {
    if (it + sizeof(DataFrameHeader) < e) {
      DataFrameHeader const& dfh = *dataframeheader_cast(&(*it));
      // check syncbyte
      if (!testDFHSync(dfh)) {
        throw common::transport::parser_error("found wrong syncbyte");
      }
      // TODO: do you want any other check?
      size_t const size_of_payload = getDFHPayloadSize(dfh);
      auto const df_end = it + sizeof(DataFrameHeader) + size_of_payload;
      if (df_end < e) {
        if (!subsequent(dfh)) {  // WARNING: now you are ignoring all subsequent
                                 // dataframes
          dataframes.emplace_back(it, df_end);
        }
        it = df_end;
      } else {
        searching = false;
      }
    } else {
      searching = false;
    }
  }
  if (!dataframes.empty()) {
    m_engine.post(std::bind(
        &dataframes_manager::timeslices_parser, this, std::move(dataframes)));
  }
  return std::distance(b, it);
}

void dataframes_manager::timeslices_parser(
    std::vector<common::transport::buffer> const&
        dataframes)  // CHECK: does it work correctly with const& ?
{
  // acquire the lock
  std::unique_lock<std::mutex> mlock(m_mutex);
  // append new dataframes
  m_dataframes.insert(
      std::end(m_dataframes), std::begin(dataframes), std::end(dataframes));
  // sort all the dataframes
  std::sort(std::begin(m_dataframes),
            std::end(m_dataframes),
            [](common::transport::buffer const& a,
               common::transport::buffer const& b) -> bool {
              DataFrameHeader const& dfh_a = *dataframeheader_cast(&a.front());
              DataFrameHeader const& dfh_b = *dataframeheader_cast(&b.front());
              return getDFHFullTime(dfh_a) < getDFHFullTime(dfh_b);
            });
  auto it = std::begin(m_dataframes);
  while (it != std::end(m_dataframes)) {
    // get current timeslice id
    uint64_t const ts_id = getTimesliceId(*dataframeheader_cast(&it->front()),
                                          boost::chrono::milliseconds(200));
    // search for timeslice completion
    it = std::find_if(
        std::begin(m_dataframes),
        std::end(m_dataframes),
        [&ts_id](common::transport::buffer const& buffer) {
          return ts_id != getTimesliceId(*dataframeheader_cast(&buffer.front()),
                                         boost::chrono::milliseconds(200));
        });
    if (it != std::end(m_dataframes)) {
      LOG_DEBUG << "Completed timeslice " << ts_id << " with "
                << std::distance(std::begin(m_dataframes), it) << " dataframes";
      it = m_dataframes.erase(std::begin(m_dataframes), it);
      LOG_DEBUG << "Next is "
                << getTimesliceId(*dataframeheader_cast(&it->front()),
                                  boost::chrono::milliseconds(200));
    }
  }
  // ERROR: this doesn't work due to the unordered hits from different pmts (so
  // you can consider closed a timeslice only if you receive an hit of the new
  // timeslice for each pmt)
}

}  // namespace ru
