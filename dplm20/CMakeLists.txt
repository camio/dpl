cmake_minimum_required(VERSION 3.6)
project(dplm20)

find_package(GTest REQUIRED)

add_library(dplm20 dplm20_overload.cpp)
target_include_directories(dplm20 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(dplm20_overload.t dplm20_overload.t.cpp)
target_link_libraries(dplm20_overload.t dplm20 GTest::GTest)
add_test(NAME dplm20_overload.t COMMAND dplm20_overload.t)
