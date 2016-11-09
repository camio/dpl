cmake_minimum_required(VERSION 3.6)
project(dplbbp)

if(NOT TARGET dplm17)
  add_subdirectory(../dplm17 ${CMAKE_BINARY_DIR}/dpl/dplm17)
endif()

if(NOT TARGET dplmrts)
  add_subdirectory(../dplmrts ${CMAKE_BINARY_DIR}/dpl/dplmrts)
endif()

find_package(GTest REQUIRED)

add_library(dplbbp
  dplbbp_anypromise.cpp
  dplbbp_promise.cpp
)
target_include_directories(dplbbp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dplbbp PUBLIC
  dplm17
  dplmrts
)

add_executable(dplbbp_promise.t dplbbp_promise.t.cpp)
target_link_libraries(dplbbp_promise.t dplbbp dplm17 GTest::GTest)
