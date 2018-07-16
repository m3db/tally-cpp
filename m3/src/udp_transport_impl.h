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

#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

#include "boost/asio.hpp"
#include "thrift/transport/TTransport.h"
#include "thrift/transport/TVirtualTransport.h"

#include "m3/udp_transport.h"

namespace m3 {

class TUDPTransport::Impl : public apache::thrift::transport::TTransport {
 public:
  Impl(const std::string &host, uint16_t port, TUDPTransport::Kind kind,
       uint16_t max_packet_size);

  ~Impl();

  // Ensure the class is non-copyable.
  Impl(const Impl &) = delete;

  Impl &operator=(const Impl &) = delete;

  // Methods to implement the TTransport interface.
  bool isOpen() override;

  void open() override;

  void close() override;

  uint32_t read_virt(uint8_t *buf, uint32_t len) override;

  void write_virt(const uint8_t *buf, uint32_t len) override;

  uint32_t writeEnd() override;

  void flush() override;

  // port returns the local port that the transport is associated with.
  uint16_t port();

 private:
  void read_async();

  void write_async();

  const std::string host_;
  const uint16_t port_;
  const TUDPTransport::Kind kind_;

  // All of the following fields must be accessed while holding the mutex.
  std::mutex mutex_;

  std::thread thread_;
  boost::asio::io_context io_context_;
  std::unique_ptr<boost::asio::ip::udp::socket> socket_;

  std::condition_variable submit_cv_;
  std::condition_variable receive_cv_;

  bool open_;
  bool in_progress_;

  std::vector<uint8_t> main_buffer_;
  std::vector<uint8_t> help_buffer_;
};

}  // namespace m3
