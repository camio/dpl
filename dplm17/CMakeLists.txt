cmake_minimum_required(VERSION 3.6)
project(dplm17)

add_library(dplm17
  dplm17_variant.h
  dplm17_variant.cpp
)
target_include_directories(dplm17 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
