cmake_minimum_required(VERSION 3.6)
project(dplmrts)

find_package(GTest REQUIRED)

add_library(dplmrts
  dplmrts_anytuple.h
  dplmrts_anytuple.cpp
#   dplmrts_invocable.h
#   dplmrts_invocable.cpp
#   dplmrts_invocablearchetype.h
#   dplmrts_invocablearchetype.cpp
)
target_include_directories(dplmrts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(dplmrts_anytuple.t dplmrts_anytuple.t.cpp)
target_link_libraries(dplmrts_anytuple.t dplmrts GTest::GTest)
add_test(NAME dplmrts_anytuple.t COMMAND dplmrts_anytuple.t)

# add_executable(dplmrts_invocable.t dplmrts_invocable.t.cpp)
# target_link_libraries(dplmrts_invocable.t dplmrts GTest::GTest)
# add_test(NAME dplmrts_invocable.t COMMAND dplmrts_invocable.t)
# 
# add_executable(dplmrts_invocablearchetype.t dplmrts_invocablearchetype.t.cpp)
# target_link_libraries(dplmrts_invocablearchetype.t dplmrts GTest::GTest)
# add_test(NAME dplmrts_invocablearchetype.t COMMAND dplmrts_invocablearchetype.t)

# ----------------------------------------------------------------------------
# Copyright 2017 Bloomberg Finance L.P.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
