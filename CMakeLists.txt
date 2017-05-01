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
