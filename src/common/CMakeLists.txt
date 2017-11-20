
include_directories(
  ${BDX_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
)

add_library(
  common
  SHARED
  log.cpp
  transport/stream_recv_client.cpp
)

target_link_libraries(
  common
  ${Boost_LIBRARIES}
)

target_compile_features(common PUBLIC cxx_std_11)