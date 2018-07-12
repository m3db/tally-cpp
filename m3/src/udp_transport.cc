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

#include "m3/udp_transport.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "boost/asio.hpp"

#include "m3/src/udp_transport_impl.h"

using apache::thrift::transport::TTransportException;
using boost::asio::ip::udp;

namespace tally {

namespace m3 {

TUDPTransport::TUDPTransport(const std::string &host, uint16_t port,
                             TUDPTransport::Kind kind, uint16_t max_packet_size)
    : impl_(new TUDPTransport::Impl(host, port, kind, max_packet_size)) {}

// Generate the default destructor here in the class file since the destructor
// of the Impl class has now been defined.
TUDPTransport::~TUDPTransport() = default;

bool TUDPTransport::isOpen() { return impl_->isOpen(); }

void TUDPTransport::open() { impl_->open(); }

void TUDPTransport::close() { impl_->close(); }

uint32_t TUDPTransport::read_virt(uint8_t *buf, uint32_t len) {
  return impl_->read_virt(buf, len);
}

void TUDPTransport::write_virt(const uint8_t *buf, uint32_t len) {
  impl_->write_virt(buf, len);
}

uint32_t TUDPTransport::writeEnd() { return impl_->writeEnd(); }

void TUDPTransport::flush() { impl_->flush(); }

uint16_t TUDPTransport::port() { return impl_->port(); }

}  // namespace m3

}  // namespace tally
