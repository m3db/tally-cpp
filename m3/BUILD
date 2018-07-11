cc_library(
    name = "m3",
    srcs = glob([
        "src/*.h",
    ]) + glob([
        "src/*.cc",
    ]) + glob([
        "include/m3/thrift/*.cpp",
    ]),
    hdrs = glob([
        "include/m3/*.h",
    ]) + glob([
        "include/m3/thrift/*.h",
    ]),
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
