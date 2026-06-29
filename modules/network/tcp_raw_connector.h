/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_NETWORK_TCP_RAW_CONNECTOR_H_20260616
#define TBOX_NETWORK_TCP_RAW_CONNECTOR_H_20260616

#include "tcp_connector.h"

namespace tbox {
namespace network {

//! 原始 TCP 连接器（无 TLS）
//! TCP 连接成功后立即创建 TcpRawConnection 并触发 connected callback
class TcpRawConnector : public TcpConnector {
  public:
    explicit TcpRawConnector(event::Loop *wp_loop);

  protected:
    virtual TcpConnection* createConnection(event::Loop *wp_loop, SocketFd fd, const SockAddr &peer_addr) override;
    virtual void onTcpConnected(SocketFd fd, const SockAddr &peer_addr) override;
};

}
}
#endif //TBOX_NETWORK_TCP_RAW_CONNECTOR_H_20260616
