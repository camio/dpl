cc_test(
    name = "dpl_promise_test",
    srcs = ["dplp_promise.t.cpp"],
    deps = [":dplp_promise", "@gtest//:main"],    
)

cc_library(
    name = "dplp_promise",
    srcs = [
        "dplp_anypromise.cpp",        
        "dplp_promise.cpp",
        "dplp_promisestate.cpp",        
        "dplp_promisestateimp.cpp",
        "dplp_promisestateimputil.cpp",
        "dplp_resolver.cpp",
    ],
    hdrs = [
        "dplp_anypromise.h",
        "dplp_promise.h",
        "dplp_promisestate.h",
        "dplp_promisestateimp.h",
        "dplp_promisestateimputil.h",
        "dplp_resolver.h",
    ],
    visibility = ["//visibility:public"],
)
