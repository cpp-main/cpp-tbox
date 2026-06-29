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
#include "tcp_raw_connection.h"

#include <tbox/base/log.h>

#undef  MODULE_ID
#define MODULE_ID "tbox.tcp"

namespace tbox {
namespace network {

TcpRawConnection::TcpRawConnection(event::Loop *wp_loop, SocketFd fd, const SockAddr &peer_addr)
  : TcpConnection(wp_loop, peer_addr)
{
    sp_buffered_fd_ = new BufferedFd(wp_loop);
    sp_buffered_fd_->initialize(fd);
    setupBufferedFd();
}

bool TcpRawConnection::doDisconnect()
{
    sp_buffered_fd_->disable();

    BufferedFd *tmp = nullptr;
    std::swap(tmp, sp_buffered_fd_);

    wp_loop_->runNext(
        [tmp] { CHECK_DELETE_OBJ(tmp); },
        "TcpRawConnection::doDisconnect, delete tmp"
    );

    return true;
}

bool TcpRawConnection::doShutdown(int howto)
{
    SocketFd socket_fd(sp_buffered_fd_->fd());
    return socket_fd.shutdown(howto) == 0;
}

}
}
