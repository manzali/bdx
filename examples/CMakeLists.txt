
include_directories(
  ${BDX_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
)

add_executable(
  logging
  logging.cpp
)

target_link_libraries(
  logging
  common
  ${Boost_LIBRARIES}
)

add_executable(
  scheduler
  scheduler.cpp
)

target_link_libraries(
  scheduler
  common
  ${Boost_LIBRARIES}
)

target_compile_features(scheduler PUBLIC cxx_std_11)

add_executable(
  futures
  futures.cpp
)

target_link_libraries(
  futures
  common
  ${Boost_LIBRARIES}
)

target_compile_features(futures PUBLIC cxx_std_11)