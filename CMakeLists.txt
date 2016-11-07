cmake_minimum_required(VERSION 3.6)

project(dpl)

if(NOT TARGET dpl_bbp)
  add_subdirectory(../dpl/bbp ${CMAKE_BINARY_DIR}/dpl/bbp)
endif()

if(NOT TARGET dpl_m17)
  add_subdirectory(../dpl/m17 ${CMAKE_BINARY_DIR}/dpl/m17)
endif()

add_custom_target(dpl DEPENDS dpl_bbp dpl_m17)
