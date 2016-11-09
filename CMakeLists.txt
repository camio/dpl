cmake_minimum_required(VERSION 3.6)

project(dpl)

if(NOT TARGET dplbbp)
  add_subdirectory(dplbbp ${CMAKE_BINARY_DIR}/dpl/dplbbp)
endif()

if(NOT TARGET dplm17)
  add_subdirectory(dplm17 ${CMAKE_BINARY_DIR}/dpl/dplm17)
endif()

if(NOT TARGET dplmrts)
  add_subdirectory(dplmrts ${CMAKE_BINARY_DIR}/dpl/dplmrts)
endif()

add_custom_target(dpl DEPENDS dplbbp dplmrts)
