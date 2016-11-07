cmake_minimum_required(VERSION 3.6)
project(dpl_m17)

add_library(dpl_m17 INTERFACE)
target_include_directories(dpl_m17 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../)
