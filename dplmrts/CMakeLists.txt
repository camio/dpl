cmake_minimum_required(VERSION 3.6)
project(dplmrts)

if(NOT TARGET dplm17)
  add_subdirectory(../dplm17 ${CMAKE_BINARY_DIR}/dpl/dplm17)
endif()

find_package(GTest REQUIRED)

add_library(dplmrts
  dplmrts_anytuple.cpp
  dplmrts_invocable.cpp
  dplmrts_invocablearchetype.cpp
)
target_include_directories(dplmrts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
