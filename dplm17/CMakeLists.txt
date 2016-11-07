cmake_minimum_required(VERSION 3.6)
project(dplm17)

add_library(dplm17 INTERFACE)
target_include_directories(dplm17 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
