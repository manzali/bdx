include_directories(
  ${BDX_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
)

add_executable(
  ru
  dataframes_manager.cpp
  main.cpp
)

target_link_libraries(
  ru
  common
  ${Boost_LIBRARIES}
)

target_compile_features(ru PUBLIC cxx_std_11)