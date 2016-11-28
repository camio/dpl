cmake_minimum_required(VERSION 3.6)
project(dplp)

if(NOT TARGET dplm17)
  add_subdirectory(../dplm17 ${CMAKE_BINARY_DIR}/dpl/dplm17)
endif()

if(NOT TARGET dplm20)
  add_subdirectory(../dplm20 ${CMAKE_BINARY_DIR}/dpl/dplm20)
endif()

if(NOT TARGET dplmrts)
  add_subdirectory(../dplmrts ${CMAKE_BINARY_DIR}/dpl/dplmrts)
endif()

find_package(GTest REQUIRED)

add_library(dplp
  dplp_anypromise.cpp
  dplp_promise.cpp
  dplp_promisestate.cpp
  dplp_promisestateimp.cpp
  dplp_promisestateimputil.cpp
  dplp_resolver.cpp
)
target_include_directories(dplp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dplp PUBLIC
  dplm17
  dplm20
  dplmrts
)

add_executable(dplp_anypromise.t dplp_anypromise.t.cpp)
target_link_libraries(dplp_anypromise.t dplp GTest::GTest)
add_test(NAME dplp_anypromise.t COMMAND dplp_anypromise.t)

add_executable(dplp_promise.t dplp_promise.t.cpp)
target_link_libraries(dplp_promise.t dplp dplm17 GTest::GTest)
add_test(NAME dplp_promise.t COMMAND dplp_promise.t)
