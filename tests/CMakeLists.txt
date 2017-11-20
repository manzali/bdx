
enable_testing() # this must be added before adding tests

include_directories(
  ${BDX_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
)

add_executable(
  t_json
  t_json.cpp
)

target_link_libraries(
  t_json
  ${Boost_LIBRARIES}
)

target_compile_features(t_json PUBLIC cxx_std_11)

add_executable(
  t_pointer_cast
  t_pointer_cast.cpp
)

target_link_libraries(
  t_pointer_cast
  ${Boost_LIBRARIES}
)

target_compile_features(t_pointer_cast PUBLIC cxx_std_11)

add_test(t_json t_json)
add_test(t_pointer_cast t_pointer_cast)

add_custom_target(
  check COMMAND ${CMAKE_CTEST_COMMAND}  --verbose
  DEPENDS
  t_json
  t_pointer_cast
)

# In order to run tests use the command "make check"