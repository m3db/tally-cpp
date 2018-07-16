# tally-cpp

Fast, buffered, hierarchical stats collection in C++.

## Abstract

Tally provides a common interface for emitting metrics, while letting you not worry about the
velocity of metrics emission.

By default it buffers counters, gauges and histograms at a specified interval, but does not
buffer timer values.  This is primarily so timer values can have all their values sampled
if desired and if not they can be sampled as summaries or histograms independently by a reporter.

## Structure

- **Scope**: Keeps track of metrics and their common metadata.
- **Metrics**: Counters, Gauges, Timers and Histograms.
- **Reporter**: Implemented by you. Forwards aggregated values from a scope to your metrics
ingestion pipeline. A default implementation for emitting tagged metrics using Thrift is provided.

### Usage

```c++
#include <chrono>
#include <unordered_map>
#include <memory>
#include <string>
#include <thread>

#include <m3/reporter.h>
#include <m3/reporter_builder.h>
#include <tally/buckets.h>
#include <tally/scope.h>
#include <tally/scope_builder.h>

int main(int argc, char** argv) {
  // Create a reporter that will be used to emit your metrics.
  auto reporter =
      m3::ReporterBuilder()
          .host("127.0.0.1")
          .port(9052)
          .common_tags({{"service", "statsdex"}, {"env", "production"}})
          .Build();

  // Create a scope which serves as a registry for your metrics.
  auto scope = tally::ScopeBuilder().reporter(reporter).ReportEvery(
      std::chrono::seconds(1));

  // Create your metrics.
  auto counter = scope->Counter("my_counter");
  auto gauge = scope->Gauge("my_gauge");
  auto timer = scope->Timer("my_timer");
  auto histogram = scope->Histogram("my_histogram",
                                    tally::Buckets::LinearValues(0.0, 1.0, 10));

  for (;;) {
    // Update your metrics.
    counter->Inc(1);
    gauge->Update(5.0);
    timer->Record(std::chrono::nanoseconds(3000000));
    histogram->Record(8.0);

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
```

## Building

[Bazel](https://bazel.build/) is used to build `tally-cpp`. In addition to Bazel, `tally-cpp`
also requires GNU build tools:

  - autoconf 2.65
  - automake 1.13
  - libtool 1.5.24

This requirement stems from the fact that `tally-cpp` depends on
[Apache thrift](https://github.com/apache/thrift) for the default reporter implementation. Thrift
uses CMake as its build system and has some pre-build steps which generate necessary header
files. We use Bazel's [`repository_rules`](https://docs.bazel.build/versions/master/skylark/repository_rules.html)
feature to ensure these pre-build commands are run after fetching the Thrift source code, following
the approach taken by Envoy and discussed in
[this blog post](https://blog.envoyproxy.io/external-c-dependency-management-in-bazel-dd37477422f5).

To use tally in your project you'll need to first at it as a dependency to your project. First
add the following to your `WORKSPACE` file:

```python
workspace(name = "com_github_jeromefroe_tally_cpp_example")

http_archive(
    name = "com_github_m3db_tally_cpp",
    strip_prefix = "tally-cpp-master",
    urls = ["https://github.com/m3db/tally-cpp/archive/master.zip"],
)

load("@com_github_m3db_tally_cpp//:repositories.bzl", "tally_cpp_repositories")

tally_cpp_repositories()

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")

boost_deps()
```

When you call `tally_cpp_repositories()` in your WORKSPACE file you'll introduce the following
dependencies to your project:

  - `load_com_google_googletest()` for Google gtest
  - `load_com_github_nelhage_rules_boost()` for Boost
  - `load_org_apache_thrift()` for Apache Thrift

In addition, you also need to load and call `boost_deps()` to add the necessary boost libraries.

Once your `WORKSPACE` file is setup, you can this library as a dependency to your own `BUILD`
file. For example:

```python
cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [
        "@com_github_m3db_tally_cpp//m3",
        "@com_github_m3db_tally_cpp//tally",
    ],
)
```

## Contributing

To check out the repo and build the library:

```
bazel build //...     # build everything
bazel build //tally   # build just the tally library
```

To run the unit tests:

```
bazel test //...            # run all unit tests
bazel test //m3/tests:unit  # run just the m3 tests
```

We try to adhere to the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
