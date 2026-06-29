/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S E  /  \/ \
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
#include "tcp_tls_connection.h"

#include <tbox/base/log.h>

#undef  MODULE_ID
#define MODULE_ID "tbox.tcp_tls"

namespace tbox {
namespace network {

TcpTlsConnection::TcpTlsConnection(event::Loop *wp_loop, SocketFd fd, const SockAddr &peer_addr, SSL *ssl)
  : TcpConnection(wp_loop, peer_addr)
{
    //! 创建 BufferedSslFd 并初始化
    auto *ssl_fd = new BufferedSslFd(wp_loop);
    ssl_fd->initialize(fd, ssl);
    sp_buffered_fd_ = ssl_fd;
    setupBufferedFd();
}

bool TcpTlsConnection::doDisconnect()
{
    //! 先尝试 SSL_shutdown（发送 close_notify）
    if (sp_buffered_fd_ != nullptr) {
        //! 获取 BufferedSslFd 中的 SSL 对象
        //! 通过 fd() 可以获取底层 fd，但我们需要 SSL 对象来 shutdown
        //! BufferedSslFd 的析构函数会处理 SSL_shutdown 和 SSL_free
        //! 所以这里只需要 disable 和 delete BufferedSslFd 即可
        //! 但为了优雅关闭，先调用一次 SSL_shutdown
        //! 注意：由于 SSL 已在 BufferedSslFd 中，我们需要另一种方式

        //! 禁用事件驱动，停止 I/O
        sp_buffered_fd_->disable();

        BufferedFd *tmp = nullptr;
        std::swap(tmp, sp_buffered_fd_);

        //! 延后删除，让 SSL_shutdown 在析构中完成
        wp_loop_->runNext(
            [tmp] { CHECK_DELETE_OBJ(tmp); },
            "TcpTlsConnection::doDisconnect, delete tmp"
        );
    }

    return true;
}

bool TcpTlsConnection::doShutdown(int howto)
{
    //! TLS 不支持半关闭的 shutdown（SSL 层面）
    //! 只能对底层 socket 执行 shutdown
    if (sp_buffered_fd_ != nullptr) {
        SocketFd socket_fd(sp_buffered_fd_->fd());
        return socket_fd.shutdown(howto) == 0;
    }
    return false;
}

}
}
