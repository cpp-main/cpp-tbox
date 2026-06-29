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
#ifndef TBOX_NETWORK_TCP_RAW_CONNECTION_H_20260616
#define TBOX_NETWORK_TCP_RAW_CONNECTION_H_20260616

#include "tcp_connection.h"

namespace tbox {
namespace network {

//! 原始 TCP 连接（无 TLS）
//! 使用 BufferedFd 进行普通 socket I/O
class TcpRawConnection : public TcpConnection {
  public:
    explicit TcpRawConnection(event::Loop *wp_loop, SocketFd fd, const SockAddr &peer_addr);

  protected:
    virtual bool doDisconnect() override;
    virtual bool doShutdown(int howto) override;
};

}
}
#endif //TBOX_NETWORK_TCP_RAW_CONNECTION_H_20260616
