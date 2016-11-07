cmake_minimum_required(VERSION 3.6)
project(dpl_bbp)

find_package(GTest REQUIRED)

add_library(dpl_bbp INTERFACE)
target_include_directories(dpl_bbp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../)

add_executable(dpl_bbp_promise.t promise.t.cpp)
target_link_libraries(dpl_bbp_promise.t dpl_bbp GTest::GTest)
