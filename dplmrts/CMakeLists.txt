cmake_minimum_required(VERSION 3.6)
project(dplmrts)

find_package(GTest REQUIRED)

add_library(dplmrts
  dplmrts_anytuple.cpp
  dplmrts_invocable.cpp
  dplmrts_invocablearchetype.cpp
)
target_include_directories(dplmrts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(dplmrts_anytuple.t dplmrts_anytuple.t.cpp)
target_link_libraries(dplmrts_anytuple.t dplmrts GTest::GTest)
add_test(NAME dplmrts_anytuple.t COMMAND dplmrts_anytuple.t)

add_executable(dplmrts_invocable.t dplmrts_invocable.t.cpp)
target_link_libraries(dplmrts_invocable.t dplmrts GTest::GTest)
add_test(NAME dplmrts_invocable.t COMMAND dplmrts_invocable.t)

add_executable(dplmrts_invocablearchetype.t dplmrts_invocablearchetype.t.cpp)
target_link_libraries(dplmrts_invocablearchetype.t dplmrts GTest::GTest)
add_test(NAME dplmrts_invocablearchetype.t COMMAND dplmrts_invocablearchetype.t)
