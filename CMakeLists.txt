cmake_minimum_required(VERSION 3.6)
project(dpl)

if(NOT TARGET dplp)
  add_subdirectory(dplp ${CMAKE_BINARY_DIR}/dpl/dplp)
endif()

if(NOT TARGET dplm17)
  add_subdirectory(dplm17 ${CMAKE_BINARY_DIR}/dpl/dplm17)
endif()

if(NOT TARGET dplm20)
  add_subdirectory(dplm20 ${CMAKE_BINARY_DIR}/dpl/dplm20)
endif()

if(NOT TARGET dplmrts)
  add_subdirectory(dplmrts ${CMAKE_BINARY_DIR}/dpl/dplmrts)
endif()

add_custom_target(dpl DEPENDS
  dplp
  dplm17
  dplmrts
)
