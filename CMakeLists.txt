cmake_minimum_required(VERSION 3.6)
project(dpl)

include(cmsi_import)

cmsi_importp(dplp)
cmsi_importp(dplm17)
cmsi_importp(dplm20)
cmsi_importp(dplmrts)

add_custom_target(dpl DEPENDS
  dplp
  dplm17
  dplm20
  dplmrts
)

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
