cmake_minimum_required(VERSION 3.6)
project(dpl_bbp)

if(NOT TARGET dpl_m17)
  add_subdirectory(../../dpl/m17 ${CMAKE_BINARY_DIR}/dpl/m17)
endif()

find_package(GTest REQUIRED)

add_library(dpl_bbp INTERFACE)
target_include_directories(dpl_bbp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../)
target_link_libraries(dpl_bbp INTERFACE dpl_m17)

add_executable(dpl_bbp_promise.t promise.t.cpp)
target_link_libraries(dpl_bbp_promise.t dpl_bbp dpl_m17 GTest::GTest)
