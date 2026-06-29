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
#ifndef TBOX_NETWORK_TCP_TLS_CONNECTION_H_20260616
#define TBOX_NETWORK_TCP_TLS_CONNECTION_H_20260616

#include <openssl/ssl.h>

#include <tbox/network/tcp_connection.h>
#include "buffered_ssl_fd.h"

namespace tbox {
namespace network {

//! TLS 加密 TCP 连接
//! 使用 BufferedSslFd 进行 SSL I/O
//! SSL 握手由 TcpTlsConnector/TcpTlsAcceptor 在创建本对象之前完成
class TcpTlsConnection : public TcpConnection {
  public:
    //! 构造函数，传入已完成握手的 SSL 对象
    //! 注意：本对象接管 SSL 的生命周期，析构时会 SSL_free()
    explicit TcpTlsConnection(event::Loop *wp_loop, SocketFd fd, const SockAddr &peer_addr, SSL *ssl);

  protected:
    virtual bool doDisconnect() override;
    virtual bool doShutdown(int howto) override;
};

}
}
#endif //TBOX_NETWORK_TCP_TLS_CONNECTION_H_20260616
