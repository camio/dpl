cmake_minimum_required(VERSION 3.6)
project(dplp)

include(cmsi_import)

cmsi_importp(dplm17)
cmsi_importp(dplm20)
cmsi_importp(dplmrts)

find_package(GTest REQUIRED)

add_library(dplp
  dplp_anypromise.h
  dplp_anypromise.cpp
  dplp_promise.h
  dplp_promise.cpp
  dplp_promisestate.h
  dplp_promisestate.cpp
  dplp_promisestateimp.h
  dplp_promisestateimp.cpp
  dplp_promisestateimputil.h
  dplp_promisestateimputil.cpp
  dplp_resolver.h
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

add_executable(dplp_resolver.t dplp_resolver.t.cpp)
target_link_libraries(dplp_resolver.t dplp GTest::GTest)
add_test(NAME dplp_resolver.t COMMAND dplp_resolver.t)

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
