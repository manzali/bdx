cmake_minimum_required(VERSION 3.6)

project(BDX CXX)

message(STATUS "Using ${CMAKE_CXX_COMPILER_ID} compiler")

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
find_package(Boost 1.65 REQUIRED COMPONENTS system log program_options)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)