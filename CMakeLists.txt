cmake_minimum_required(VERSION 3.6)

project(dpl)

if(NOT TARGET dpl_bbp)
  add_subdirectory(../dpl/bbp ${CMAKE_BINARY_DIR}/dpl/bbp)
endif()

add_custom_target(dpl DEPENDS dpl_bbp)
