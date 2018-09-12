cc_test(
    name = "unit",
    srcs = [
        "mock_handler.h",
        "mock_server.h",
        "reporter_test.cc",
    ],
    copts = ["-Iexternal/googletest/include"],
    linkstatic = 1,
    deps = [
        "//m3",
        "//tally",
        "@com_google_googletest//:gtest_main",
    ],
)
