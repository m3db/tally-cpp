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

#include "m3/src/udp_transport_impl.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "boost/asio.hpp"

#include "m3/udp_transport.h"

using apache::thrift::transport::TTransportException;
using boost::asio::ip::udp;

namespace tally {

namespace m3 {

TUDPTransport::Impl::Impl(const std::string &host, uint16_t port,
                          TUDPTransport::Kind kind, uint16_t max_packet_size)
    : host_(host), port_(port), kind_(kind), open_(false), in_progress_(false) {
  // Reserve space for 5 packets in the buffers.
  main_buffer_.reserve(max_packet_size * 5);
  help_buffer_.reserve(max_packet_size * 5);
}

TUDPTransport::Impl::~Impl() { close(); }

bool TUDPTransport::Impl::isOpen() {
  std::lock_guard<std::mutex> lock(mutex_);
  return open_;
}

void TUDPTransport::Impl::open() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (open_) {
    return;
  }

  udp::resolver resolver(io_context_);
  udp::resolver::results_type endpoints =
      resolver.resolve(udp::v4(), host_, std::to_string(port_));

  if (endpoints.size() == 0) {
    throw TTransportException(TTransportException::BAD_ARGS);
  }
  auto endpoint = *endpoints.begin();

  socket_ =
      std::unique_ptr<udp::socket>(new udp::socket(io_context_, udp::v4()));

  if (kind_ == TUDPTransport::Kind::Server) {
    socket_->bind(endpoint);
    thread_ = std::thread([this] {
      read_async();
      io_context_.run();
    });
  } else {
    socket_->connect(endpoint);
    thread_ = std::thread([this] {
      write_async();
      io_context_.run();
    });
  }

  open_ = true;
}

void TUDPTransport::Impl::close() {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!open_) {
    return;
  }

  open_ = false;
  lock.unlock();

  receive_cv_.notify_one();
  submit_cv_.notify_one();

  io_context_.stop();
  thread_.join();

  socket_->close();
}

uint32_t TUDPTransport::Impl::read_virt(uint8_t *buf, uint32_t len) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!open_) {
    throw TTransportException(TTransportException::NOT_OPEN);
  }

  if (kind_ != TUDPTransport::Kind::Server) {
    throw TTransportException(
        "TUDPTransport only supports read operations when initialized as a "
        "server");
  }

  if (in_progress_) {
    throw TTransportException(
        "TUDPTransport does not support concurrent read operations");
  }
  in_progress_ = true;

  // Add predicate to wait to avoid spurious wakeups.
  receive_cv_.wait(lock, [this] { return !open_ || main_buffer_.size() != 0; });

  if (!open_) {
    in_progress_ = false;
    throw TTransportException(TTransportException::NOT_OPEN);
  }

  size_t n = std::min(static_cast<size_t>(len), main_buffer_.size());

  std::copy(main_buffer_.begin(), main_buffer_.begin() + n, buf);

  if (n == main_buffer_.size()) {
    main_buffer_.clear();
  } else {
    main_buffer_.erase(main_buffer_.begin(), main_buffer_.begin() + n);
  }

  in_progress_ = false;
  return n;
}

void TUDPTransport::Impl::read_async() {
  help_buffer_.clear();
  help_buffer_.resize(1440);
  socket_->async_receive(
      boost::asio::buffer(help_buffer_),
      [this](boost::system::error_code ec, std::size_t bytes_recvd) {
        if (!ec && bytes_recvd > 0) {
          std::unique_lock<std::mutex> lock(mutex_);
          main_buffer_.insert(main_buffer_.end(), help_buffer_.begin(),
                              help_buffer_.begin() + bytes_recvd);
          lock.unlock();

          receive_cv_.notify_one();
        } else {
          if (ec) {
            std::cerr << "Encountered error receiving Thrift UDP packet: " << ec
                      << std::endl;
          }
        }
        read_async();
      });
}

void TUDPTransport::Impl::write_virt(const uint8_t *buf, uint32_t len) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!open_) {
    throw TTransportException(TTransportException::NOT_OPEN);
  }

  if (kind_ != TUDPTransport::Kind::Client) {
    throw TTransportException(
        "TUDPTransport only supports write operations when initialized as a "
        "server");
  }

  main_buffer_.insert(main_buffer_.end(), buf, buf + len);
}

uint32_t TUDPTransport::Impl::writeEnd() { return 0; }

void TUDPTransport::Impl::flush() {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!open_) {
    throw TTransportException(TTransportException::NOT_OPEN);
  }

  if (kind_ != TUDPTransport::Kind::Client) {
    throw TTransportException(
        "TUDPTransport only supports write operations when initialized as a "
        "server");
  }

  if (in_progress_) {
    throw TTransportException(
        "TUDPTransport does not support concurrent send operations");
  }

  auto size = main_buffer_.size();
  if (size == 0) {
    return;
  }

  if (size > MAX_UDP_PACKET_SIZE) {
    main_buffer_.clear();

    std::stringstream msg;
    msg << "M3 Thrift UDP Transport buffer size  " << size
        << " exceeds maximum UDP packet size" << std::endl;
    throw TTransportException(TTransportException::UNKNOWN, msg.str());
  }

  in_progress_ = true;

  submit_cv_.notify_one();

  receive_cv_.wait(lock);

  in_progress_ = false;

  if (!open_) {
    throw TTransportException(TTransportException::NOT_OPEN);
  }
}

void TUDPTransport::Impl::write_async() {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!open_) {
    return;
  }

  // Add predicate to wait to avoid spurious wakeup.
  submit_cv_.wait(lock, [this] { return in_progress_ || !open_; });
  if (!open_) {
    return;
  }

  help_buffer_.insert(help_buffer_.end(), main_buffer_.begin(),
                      main_buffer_.end());
  main_buffer_.clear();

  socket_->async_send(
      boost::asio::buffer(help_buffer_),
      [this](boost::system::error_code ec, std::size_t bytes_sent) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (!ec && bytes_sent > 0) {
          if (bytes_sent == help_buffer_.size()) {
            help_buffer_.clear();
          } else {
            help_buffer_.erase(help_buffer_.begin(),
                               help_buffer_.begin() + bytes_sent);
          }
        } else {
          if (ec) {
            std::cerr << "Encountered error sending Thrift UDP packet: " << ec
                      << std::endl;
          }
        }

        lock.unlock();

        receive_cv_.notify_one();
        write_async();
      });
}

uint16_t TUDPTransport::Impl::port() {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!open_) {
    throw TTransportException(TTransportException::NOT_OPEN);
  }

  auto endpoint = socket_->local_endpoint();
  return endpoint.port();
}

}  // namespace m3

}  // namespace tally
