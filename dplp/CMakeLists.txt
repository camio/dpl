cmake_minimum_required(VERSION 3.6)
project(dplp)

if(NOT TARGET dplm17)
  add_subdirectory(../dplm17 ${CMAKE_BINARY_DIR}/dpl/dplm17)
endif()

if(NOT TARGET dplmrts)
  add_subdirectory(../dplmrts ${CMAKE_BINARY_DIR}/dpl/dplmrts)
endif()

find_package(GTest REQUIRED)

add_library(dplp
  dplp_anypromise.cpp
  dplp_promise.cpp
  dplp_resolver.cpp
)
target_include_directories(dplp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dplp PUBLIC
  dplm17
  dplmrts
)

add_executable(dplp_promise.t dplp_promise.t.cpp)
target_link_libraries(dplp_promise.t
  dplp
  dplm17
  GTest::GTest
)