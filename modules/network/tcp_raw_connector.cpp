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
#include "tcp_raw_connector.h"
#include "tcp_raw_connection.h"

#include <tbox/base/log.h>

#undef  MODULE_ID
#define MODULE_ID "tbox.tcp"

namespace tbox {
namespace network {

TcpRawConnector::TcpRawConnector(event::Loop *wp_loop)
  : TcpConnector(wp_loop)
{ }

TcpConnection* TcpRawConnector::createConnection(event::Loop *wp_loop, SocketFd fd, const SockAddr &peer_addr)
{
    return new TcpRawConnection(wp_loop, fd, peer_addr);
}

void TcpRawConnector::onTcpConnected(SocketFd fd, const SockAddr &peer_addr)
{
    //! TCP 连接成功后，立即创建 TcpRawConnection 并触发回调
    if (connected_cb_) {
        auto sp_conn = createConnection(wp_loop_, fd, peer_addr);
        sp_conn->enable();
        ++cb_level_;
        connected_cb_(sp_conn);
        --cb_level_;
    } else {
        LogWarn("connected callback is not set");
        //! 没有回调，需要关闭 fd
        fd.close();
    }
}

}
}
