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

#include <memory>
#include <string>

#include "thrift/transport/TTransport.h"

namespace tally {
namespace m3 {

class TUDPTransport : public apache::thrift::transport::TTransport {
 public:
  enum class Kind {
    Server,
    Client,
  };

  TUDPTransport(const std::string &host, uint16_t port,
                TUDPTransport::Kind kind, uint16_t max_packet_size);

  ~TUDPTransport();

  // Ensure the class is non-copyable.
  TUDPTransport(const TUDPTransport &) = delete;

  TUDPTransport &operator=(const TUDPTransport &) = delete;

  // Methods to implement the TTransport interface.
  bool isOpen() override;

  void open() override;

  void close() override;

  uint32_t read_virt(uint8_t *buf, uint32_t len) override;

  void write_virt(const uint8_t *buf, uint32_t len) override;

  uint32_t writeEnd() override;

  void flush() override;

  // port returns local port that the transport is associated with.
  uint16_t port();

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace m3

}  // namespace tally
