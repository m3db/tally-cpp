// Copyright (c) 2018 Uber Technologies, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#include "thrift/protocol/TCompactProtocol.h"
#include "thrift/server/TSimpleServer.h"
#include "thrift/transport/TBufferTransports.h"
#include "thrift/transport/TServerSocket.h"

#include "m3/thrift/M3.h"
#include "m3/udp_transport.h"

using apache::thrift::TProcessor;
using apache::thrift::protocol::TCompactProtocolFactory;
using apache::thrift::protocol::TProtocolFactory;
using apache::thrift::transport::TTransportException;

class MockServer {
 public:
  MockServer(const std::string& host, uint16_t port) : stopped_(false) {
    handler_ =
        apache::thrift::stdcxx::shared_ptr<MockHandler>(new MockHandler());
    processor_ = apache::thrift::stdcxx::shared_ptr<TProcessor>(
        new m3::thrift::M3Processor(handler_));
    transport_ = apache::thrift::stdcxx::shared_ptr<m3::TUDPTransport>(
        new m3::TUDPTransport(host, port, m3::TUDPTransport::Kind::Server,
                              1440));
    transport_->open();
  }

  ~MockServer() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!stopped_) {
      stop();
    }
  }

  void serve() {
    apache::thrift::stdcxx::shared_ptr<TProtocolFactory> protocolFactory(
        new TCompactProtocolFactory());
    auto protocol = protocolFactory->getProtocol(transport_);

    while (true) {
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopped_) {
          return;
        }
      }

      try {
        processor_->process(protocol, protocol, NULL);
      } catch (const TTransportException& e) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopped_) {
          return;
        }
        std::cerr << "Encountered error processing M3 metrics: " << e.what()
                  << std::endl;
      }
    }
  }

  void stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    stopped_ = true;
    transport_->close();
  }

  bool empty() { return handler_->empty(); }

  m3::thrift::MetricBatch getBatch() { return handler_->getBatch(); }

  uint16_t port() { return transport_->port(); }

 private:
  apache::thrift::stdcxx::shared_ptr<TProcessor> processor_;
  apache::thrift::stdcxx::shared_ptr<m3::TUDPTransport> transport_;
  apache::thrift::stdcxx::shared_ptr<MockHandler> handler_;

  std::mutex mutex_;
  bool stopped_;
};
