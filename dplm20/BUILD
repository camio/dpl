cc_test(
	name = "dplm20_overload_test",
	srcs = [
	     "dplm20_overload.t.cpp",
	],
	copts = ["-std=c++14"],
	deps = [":dplm20", "@gtest//:main"],
	visibility = ["//visibility:public"],
)

cc_library(
	name = "dplm20",
	hdrs = [
	     "dplm20_overload.h",
	],
	srcs = [
	     "dplm20_overload.cpp",
	],
	copts = ["-std=c++14"],
	visibility = ["//visibility:public"],
)