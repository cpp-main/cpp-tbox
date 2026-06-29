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
#include "tcp_raw_acceptor.h"
#include "tcp_raw_connection.h"

#include <tbox/base/log.h>

#undef  MODULE_ID
#define MODULE_ID "tbox.tcp"

namespace tbox {
namespace network {

TcpRawAcceptor::TcpRawAcceptor(event::Loop *wp_loop)
  : TcpAcceptor(wp_loop)
{ }

TcpConnection* TcpRawAcceptor::createConnection(event::Loop *wp_loop, SocketFd fd, const SockAddr &peer_addr)
{
    return new TcpRawConnection(wp_loop, fd, peer_addr);
}

void TcpRawAcceptor::onClientAccepted(SocketFd fd, const SockAddr &peer_addr)
{
    //! accept 后立即创建 TcpRawConnection 并触发回调
    if (new_conn_cb_) {
        auto sp_connection = createConnection(wp_loop_, fd, peer_addr);
        sp_connection->enable();
        ++cb_level_;
        new_conn_cb_(sp_connection);
        --cb_level_;
    } else {
        LogWarn("%s need connect cb", bind_addr_.toString().c_str());
        //! 没有回调，需要关闭 fd
        fd.close();
    }
}

}
}
