def load_com_google_googletest():
    native.http_archive(
        name = "com_google_googletest",
        strip_prefix = "googletest-ba96d0b1161f540656efdaed035b3c062b60e006",
        urls = [
            "https://github.com/google/googletest/archive/ba96d0b1161f540656efdaed035b3c062b60e006.zip",
        ],
    )

def load_com_github_nelhage_rules_boost():
    native.git_repository(
        name = "com_github_nelhage_rules_boost",
        commit = "aebc446ac0c02efdab3e1200e3b0221bd0ddb8da",
        remote = "https://github.com/nelhage/rules_boost",
    )

THRIFT_BUILD_SH = """
#!/bin/bash
git clone https://github.com/apache/thrift
cd thrift
# Checkout version 0.11.0. We use a git SHA instead of a tag here because the latter is mutable.
git checkout 327ebb6c2b6df8bf075da02ef45a2a034e9b79ba
./bootstrap.sh
./configure
"""

THRIFT_BUILD = """
cc_library(
    name = "org_apache_thrift",
    srcs = [
        "thrift/lib/cpp/src/thrift/TApplicationException.cpp",
        "thrift/lib/cpp/src/thrift/TOutput.cpp",
        "thrift/lib/cpp/src/thrift/async/TConcurrentClientSyncInfo.cpp",
        "thrift/lib/cpp/src/thrift/concurrency/Monitor.cpp",
        "thrift/lib/cpp/src/thrift/concurrency/Mutex.cpp",
        "thrift/lib/cpp/src/thrift/concurrency/Util.cpp",
        "thrift/lib/cpp/src/thrift/protocol/TProtocol.cpp",
        "thrift/lib/cpp/src/thrift/server/TConnectedClient.cpp",
        "thrift/lib/cpp/src/thrift/server/TServer.cpp",
        "thrift/lib/cpp/src/thrift/server/TServerFramework.cpp",
        "thrift/lib/cpp/src/thrift/server/TSimpleServer.cpp",
        "thrift/lib/cpp/src/thrift/transport/TBufferTransports.cpp",
        "thrift/lib/cpp/src/thrift/transport/TSocket.cpp",
        "thrift/lib/cpp/src/thrift/transport/TServerSocket.cpp",
        "thrift/lib/cpp/src/thrift/transport/TTransportException.cpp",
    ],
    hdrs = [
        "thrift/lib/cpp/src/thrift/config.h",
        "thrift/lib/cpp/src/thrift/stdcxx.h",
        "thrift/lib/cpp/src/thrift/thrift-config.h",
        "thrift/lib/cpp/src/thrift/Thrift.h",
        "thrift/lib/cpp/src/thrift/TApplicationException.h",
        "thrift/lib/cpp/src/thrift/TBase.h",
        "thrift/lib/cpp/src/thrift/TDispatchProcessor.h",
        "thrift/lib/cpp/src/thrift/TLogging.h",
        "thrift/lib/cpp/src/thrift/TOutput.h",
        "thrift/lib/cpp/src/thrift/TProcessor.h",
        "thrift/lib/cpp/src/thrift/TToString.h",
        "thrift/lib/cpp/src/thrift/async/TConcurrentClientSyncInfo.h",
        "thrift/lib/cpp/src/thrift/concurrency/Exception.h",
        "thrift/lib/cpp/src/thrift/concurrency/Monitor.h",
        "thrift/lib/cpp/src/thrift/concurrency/Mutex.h",
        "thrift/lib/cpp/src/thrift/concurrency/Thread.h",
        "thrift/lib/cpp/src/thrift/concurrency/Util.h",
        "thrift/lib/cpp/src/thrift/protocol/TProtocol.h",
        "thrift/lib/cpp/src/thrift/protocol/TBinaryProtocol.h",
        "thrift/lib/cpp/src/thrift/protocol/TBinaryProtocol.tcc",
        "thrift/lib/cpp/src/thrift/protocol/TCompactProtocol.h",
        "thrift/lib/cpp/src/thrift/protocol/TCompactProtocol.tcc",
        "thrift/lib/cpp/src/thrift/protocol/TVirtualProtocol.h",
        "thrift/lib/cpp/src/thrift/protocol/TProtocolException.h",
        "thrift/lib/cpp/src/thrift/server/TConnectedClient.h",
        "thrift/lib/cpp/src/thrift/server/TServer.h",
        "thrift/lib/cpp/src/thrift/server/TServerFramework.h",
        "thrift/lib/cpp/src/thrift/server/TSimpleServer.h",
        "thrift/lib/cpp/src/thrift/transport/PlatformSocket.h",
        "thrift/lib/cpp/src/thrift/transport/TTransport.h",
        "thrift/lib/cpp/src/thrift/transport/TBufferTransports.h",
        "thrift/lib/cpp/src/thrift/transport/TSocket.h",
        "thrift/lib/cpp/src/thrift/transport/TServerSocket.h",
        "thrift/lib/cpp/src/thrift/transport/TServerTransport.h",
        "thrift/lib/cpp/src/thrift/transport/TVirtualTransport.h",
        "thrift/lib/cpp/src/thrift/transport/TTransportException.h",
    ],
    linkopts = ["-pthread"],
    strip_include_prefix = "thrift/lib/cpp/src",
    visibility = ["//visibility:public"],
    deps = [
        "@boost//:config",
        "@boost//:core",
        "@boost//:format",
        "@boost//:noncopyable",
        "@boost//:numeric_conversion",
        "@boost//:scoped_array",
        "@boost//:type_traits",
        "@boost//:utility",
    ],
)
"""

def _thrift_repository_impl(ctxt):
    ctxt.file("thrift_build.sh", content = THRIFT_BUILD_SH)
    ctxt.file("BUILD", content = THRIFT_BUILD, executable = False)
    ctxt.execute(["./thrift_build.sh"])

thrift_repository = repository_rule(
    implementation = _thrift_repository_impl,
)

def load_org_apache_thrift():
    thrift_repository(
        name = "org_apache_thrift",
    )

def tally_cpp_repositories():
    load_com_google_googletest()
    load_com_github_nelhage_rules_boost()
    load_org_apache_thrift()
