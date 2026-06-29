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
#ifndef TBOX_NETWORK_TCP_TLS_CONNECTOR_H_20260616
#define TBOX_NETWORK_TCP_TLS_CONNECTOR_H_20260616

#include <openssl/ssl.h>

#include <tbox/network/tcp_connector.h>
#include <tbox/network/tls_config.h>
#include <tbox/network/socket_fd.h>

namespace tbox {
namespace network {

//! TLS 连接器
//! TCP 连接成功后，先进行 SSL 握手，握手成功后才创建 TcpTlsConnection 并触发 connected callback
//! 握手失败视为连接失败，触发重连逻辑
class TcpTlsConnector : public TcpConnector {
  public:
    explicit TcpTlsConnector(event::Loop *wp_loop, SSL_CTX *ssl_ctx, const TlsConfig &tls_config);
    ~TcpTlsConnector();

  protected:
    virtual TcpConnection* createConnection(event::Loop *wp_loop, SocketFd fd, const SockAddr &peer_addr) override;
    virtual void onTcpConnected(SocketFd fd, const SockAddr &peer_addr) override;

  private:
    //! 开始 SSL 握手
    void startSslHandshake(SocketFd fd, const SockAddr &peer_addr);
    //! SSL 握手事件处理
    void onSslHandshakeEvent(short events);
    //! SSL 握手成功
    void onSslHandshakeSuccess();
    //! SSL 握手失败
    void onSslHandshakeFail();

  private:
    SSL_CTX *ssl_ctx_ = nullptr;
    TlsConfig tls_config_;

    //! 握手期间的临时状态
    SSL *handshake_ssl_ = nullptr;
    SocketFd handshake_fd_;
    SockAddr handshake_peer_addr_;
    event::FdEvent *sp_handshake_ev_ = nullptr;
};

}
}
#endif //TBOX_NETWORK_TCP_TLS_CONNECTOR_H_20260616
