cc_library(
    name = "m3",
    srcs = glob([
        "src/*.h",
    ]) + glob([
        "src/*.cc",
    ]) + [
        "include/m3/thrift/M3.cpp",
        "include/m3/thrift/m3_constants.cpp",
        "include/m3/thrift/m3_types.cpp",
        "include/m3/thrift/operator.cpp",
    ],
    hdrs = glob([
        "include/m3/*.h",
    ]) + [
        "include/m3/thrift/M3.h",
        "include/m3/thrift/m3_constants.h",
        "include/m3/thrift/m3_types.h",
    ],
    linkstatic = 1,
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
    deps = [
        "//tally",
        "@boost//:asio",
        "@boost//:format",
        "@org_apache_thrift",
    ],
)
