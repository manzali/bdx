#ifndef COMMON_DATAFORMAT_F_DATAFORMAT_HPP
#define COMMON_DATAFORMAT_F_DATAFORMAT_HPP

#include <boost/chrono.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/cstdint.hpp>

#include <cstddef>
#include <cassert>
#include <ostream>

#include "common/dataformat/s_dataformat.hpp"

// It is possible to represent a full century time
// in unit of 5 nano seconds with a 64bit integer.
typedef boost::chrono::duration<boost::int_least64_t, boost::ratio<1, 8000> > T125usec;
typedef boost::chrono::duration<boost::int_least64_t, boost::ratio<1, 200000000> > T5nsec;
typedef boost::chrono::duration<boost::int_least64_t, boost::ratio<86400> > days;

namespace common {

namespace dataformat {

typedef T125usec coarse_time;
typedef T5nsec fine_time;

inline DataFrameHeader* dataframeheader_cast(unsigned char* pointer) {
  return static_cast<DataFrameHeader*>(static_cast<void*>(pointer));
}

inline const DataFrameHeader* dataframeheader_cast(
    unsigned char const* pointer) {
  return static_cast<DataFrameHeader const*>(static_cast<void const*>(pointer));
}

inline
bool compressed(DataFrameHeader const& header) {
  return header.ZipFlag;
}

inline
bool subsequent(DataFrameHeader const& header) {
  return header.FragFlag;
}

inline
unsigned int getDFHSampleSize(DataFrameHeader const& header) {
  return compressed(header) ? 1 : 2;
}

inline fine_time getCurrentTime() {
  // This is the time in seconds elapsed since the epoch of the 1-gen-2000 0:00
  const static boost::chrono::seconds unix_time_1_1_2000(946684800);
  return boost::chrono::duration_cast<fine_time>(
      boost::chrono::system_clock::now().time_since_epoch()
          - unix_time_1_1_2000);
}

inline
bool testDFHSync(DataFrameHeader const& header) {
  return header.SyncBytes == 21930;
}

inline
unsigned int getDFHCharge(DataFrameHeader const& header) {
  return header.Charge;
}

inline size_t getDFHPayloadSize(DataFrameHeader const& header) {
  return header.NDataSamples * getDFHSampleSize(header);
}

inline size_t getDFHNSamples(DataFrameHeader const& header) {
  return header.NDataSamples;
}

// Converts the year BCD format in binary

inline
unsigned int getDFHYears(DataFrameHeader const& header) {
  const static uint8_t mask1 = 15, mask2 = 240;
  unsigned int const r = header.Years & mask1;
  return r + ((10 * (header.Years & mask2)) >> 4);
}

// Converts the day BCD format in binary

inline
unsigned int getDFHDays(DataFrameHeader const& header) {
  const static unsigned int mask1 = 15, mask2 = 240, mask3 = 768;
  unsigned int r = header.Days & mask1;
  r += (10 * (header.Days & mask2)) >> 4;
  return r + ((100 * (header.Days & mask3)) >> 8);
}

// Time in seconds since midnight

inline fine_time getDFHDayTime(DataFrameHeader const& header) {
  return boost::chrono::seconds(header.Seconds) + T125usec(header.FrameCounter)
      + T5nsec(header.T5ns);
}

// Time in seconds since the 1-gen of this year

inline fine_time getDFHYearTime(DataFrameHeader const& header) {
  return boost::chrono::seconds(86400 * getDFHDays(header))
      + getDFHDayTime(header);
}

// Time elapsed since 1-gen-2000

inline fine_time getDFHFullTime(DataFrameHeader const& header) {
  unsigned int const years = getDFHYears(header);
  unsigned int bissextile_days = (years >> 2) + 1;

  if (years % 4 == 0) {
    --bissextile_days;
  }

  return boost::chrono::seconds(31536000 * years + 86400 * bissextile_days)
      + getDFHYearTime(header);
}

// Set the Days field. "gregorian" is a number representing the number of
// days since the start of the year. 1 Jan == 0

inline
void setDFHDays(unsigned int gregorian, DataFrameHeader& header) {
  assert(gregorian < 367);
  unsigned int const hundreds = gregorian / 100;
  unsigned int const tens = (gregorian % 100) / 10;
  unsigned int const units = gregorian % 10;
  header.Days = (hundreds << 8) + (tens << 4) + units;
}

// Set the Years field. "year" is a number representing the number of
// years since the start of the century. 2000 == 0

inline
void setDFHYears(unsigned int year, DataFrameHeader& header) {
  assert(year < 100);
  unsigned int const tens = year / 10;
  unsigned int const units = year % 10;
  header.Years = (tens << 4) + units;
}

inline
void setDFHDayTime(fine_time const& time, DataFrameHeader& header) {
  assert(time < days(1));
  boost::chrono::seconds const secs = boost::chrono::duration_cast<
      boost::chrono::seconds>(time);
  header.Seconds = secs.count();

  T125usec const boost_framecounter = boost::chrono::duration_cast<T125usec>(
      time - secs);
  header.FrameCounter = boost_framecounter.count();

  header.T5ns = (time - secs - boost_framecounter).count();
}

// Set the time since the beginning of the year.

inline
void setDFHYearTime(fine_time const& time, DataFrameHeader& header) {
  days const gregorian = boost::chrono::duration_cast<days>(time);

  setDFHDays(gregorian.count(), header);

  setDFHDayTime(time - gregorian, header);
}

// Set the time since Jan 1 2000.

inline
void setDFHFullTime(fine_time const& time, DataFrameHeader& header) {
  boost::chrono::seconds const s = boost::chrono::duration_cast<
      boost::chrono::seconds>(time);

  // Please note that boost::posix_time::seconds uses a signed 32-bit integer
  // as internal storage for the time, while boost::chrono::seconds uses a
  // least64. The following line will silently fail if `time` refers to a time
  // point which is after about the year 2068.
  // A better solution shall be found before that date.
  boost::posix_time::ptime const t(
      boost::gregorian::date(2000, boost::date_time::Jan, 1),
      boost::posix_time::seconds(s.count()));

  boost::gregorian::date const d = t.date();

  unsigned int const year = d.year() - 2000;

  setDFHYears(year, header);

  boost::posix_time::ptime const first_of_the_year(
      boost::gregorian::date(d.year(), boost::date_time::Jan, 1));

  boost::chrono::seconds const x((t - first_of_the_year).total_seconds());

  fine_time const rest = time - s;

  setDFHYearTime(x + rest, header);
}

inline uint64_t getTimesliceId(
    DataFrameHeader const& dfh,
    boost::chrono::milliseconds ts_size) {
  assert(ts_size > boost::chrono::milliseconds::zero());
  boost::chrono::milliseconds const total_ms = boost::chrono::duration_cast<
      boost::chrono::milliseconds>(getDFHFullTime(dfh));
  return total_ms.count() / ts_size.count();
}

inline size_t maxFrameSize() {
  // See the dataformat document (KM3IT_DataFrame.docx):
  // La lunghezza del data frame [...] compresa tra 8 e 44 WORD.
  // sizeof(WORD) = 2

  return 44 * 2;
}

inline std::ostream& operator <<(
    std::ostream& stream,
    DataFrameHeader const& header) {
  return stream
  // First DWORD: Sync and ID
  << " == Sync & ID  ==\n\n"
  << "SyncBytes:     "
  << header.SyncBytes
  << '\n'
  << "TowerID:       "
  << header.TowerID
  << '\n'
  << "EFCMID:        "
  << header.EFCMID
  << '\n'
  << "PMTID:         "
  << header.PMTID
  << '\n'

  // Second and third DWORD: Time
  << "\n ==   Time   ==\n\n"
  << "Years (BCD):   "
  << header.Years
  << '\n'
  << "Days (BCD):    "
  << header.Days
  << '\n'
  << "Seconds:       "
  << header.Seconds
  << '\n'
  << "FrameCounter:  "
  << header.FrameCounter
  << '\n'
  << "T5ns:          "
  << header.T5ns
  << '\n'
  << "\nYears (human): "
  << getDFHYears(header)
  << '\n'
  << "Days (human) : "
  << getDFHDays(header)
  << '\n'
  << "\nCentury time in 5 nanoseconds unit: "
  << getDFHFullTime(header)
  << '\n'

  // Forth DWORD: Hit info
  << "\n == Hit info ==\n\n"
  << "ZipFlag:       "
  << header.ZipFlag
  << '\n'
  << "FragFlag:      "
  << header.FragFlag
  << '\n'
  << "Charge:        "
  << header.Charge
  << '\n'
  << "FifoFull:      "
  << header.FifoFull
  << '\n'
  << "NDataSamples:  "
  << header.NDataSamples
  << '\n'

  << "\nPayload size:  "
  << getDFHPayloadSize(header)
  << '\n';
}

}

}

#endif
