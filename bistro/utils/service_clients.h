/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <gflags/gflags.h>
#include <memory>
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>

#include "bistro/bistro/if/gen-cpp2/common_types.h"

DECLARE_int32(thrift_connect_timeout_ms);
DECLARE_int32(thrift_send_timeout_ms);
DECLARE_int32(thrift_receive_timeout_ms);

namespace facebook { namespace bistro {

/**
 * Makes a Thrift asynchronous client for a ServiceAddress and port.
 *
 * Add proxy support here, if desired.
 */
template<typename ThriftClient>
std::shared_ptr<ThriftClient> getAsyncClientForAddress(
  folly::EventBase* event_base,
  const cpp2::ServiceAddress& addr,
  int connect_timeout_ms = 0,
  int send_timeout_ms = 0,
  int receive_timeout_ms = 0
) {
  if (connect_timeout_ms == 0) {
    connect_timeout_ms = FLAGS_thrift_connect_timeout_ms;
  }
  if (send_timeout_ms == 0) {
    send_timeout_ms = FLAGS_thrift_send_timeout_ms;
  }
  if (receive_timeout_ms == 0) {
    receive_timeout_ms = FLAGS_thrift_receive_timeout_ms;
  }
  using namespace apache::thrift;
  auto socket = folly::AsyncSocket::newSocket(
      event_base,
      *addr.ip_or_host_ref(),
      *addr.port_ref(),
      connect_timeout_ms);
  auto socketPtr = socket.get();
  auto channel = HeaderClientChannel::newChannel(std::move(socket));
  // This also sets the socket send timeout, but we overwrite it below.
  channel->setTimeout(receive_timeout_ms);
  socketPtr->setSendTimeout(send_timeout_ms);
  return std::make_shared<ThriftClient>(std::move(channel));
}

}}
