#ifndef COMMON_DATAFORMAT_S_DATAFORMAT_HPP
#define COMMON_DATAFORMAT_S_DATAFORMAT_HPP

namespace common {

namespace dataformat {

#pragma pack(push, 1)
struct DataFrameHeader {
  // First DWORD: Sync and ID
  unsigned int PMTID :4;  // 0-15
  unsigned int EFCMID :5;  // 0-31
  unsigned int TowerID :7;  // 0-127
  unsigned int SyncBytes :16;

  // Second and third DWORD: Time
  unsigned int T5ns :16;  // 5 nanoseconds counter
  unsigned int FrameCounter :13;  // 0-7999
  unsigned int Seconds :17;  // 0-86399
  unsigned int Days :10;  // BCD! 0-365 (366)
  unsigned int Years :8;  // BCD! 0-99

  // Forth DWORD: Hit info
  unsigned int NDataSamples :6;  // Number of samples
  unsigned int unused :1;
  unsigned int FifoFull :1;  // FIFO full flag
  unsigned int Charge :22;
  unsigned int FragFlag :1;  // Fragmentation flag
  unsigned int ZipFlag :1;  // Compression flag
};
#pragma pack(pop)

}

}

#endif
