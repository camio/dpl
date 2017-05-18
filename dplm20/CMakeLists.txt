cmake_minimum_required(VERSION 3.6)
project(dplm20)

find_package(GTest REQUIRED)

add_library(dplm20
  dplm20_overload.h
  dplm20_overload.cpp
)
target_include_directories(dplm20 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(dplm20_overload.t dplm20_overload.t.cpp)
target_link_libraries(dplm20_overload.t dplm20 GTest::GTest)
add_test(NAME dplm20_overload.t COMMAND dplm20_overload.t)

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
