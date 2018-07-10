cc_test(
    name = "unit",
    srcs = [
        "buckets_test.cc",
        "counter_impl_test.cc",
        "gauge_impl_test.cc",
        "histogram_impl_test.cc",
        "mock_stats_reporter.h",
        "scope_impl_test.cc",
        "timer_impl_test.cc",
    ],
    copts = ["-Iexternal/googletest/include"],
    linkstatic = 1,
    deps = [
        "//tally",
        "@com_google_googletest//:gtest_main",
    ],
)
