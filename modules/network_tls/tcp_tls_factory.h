/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *   //  E A S Y  /  \/ \
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
#ifndef TBOX_NETWORK_TCP_TLS_FACTORY_H_20260616
#define TBOX_NETWORK_TCP_TLS_FACTORY_H_20260616

#include <openssl/ssl.h>

#include <tbox/network/tcp_factory.h>
#include <tbox/network/tls_config.h>
#include <tbox/network/tls_factory_entry.h>

namespace tbox {
namespace network {

//! TLS 工厂
//! 根据 TlsRole 创建对应的 SSL_CTX，仅持有本端所需的那一个
//! kClient 角色：创建 client SSL_CTX，仅支持 createConnector
//! kServer 角色：创建 server SSL_CTX，仅支持 createAcceptor
class TcpTlsFactory : public TcpFactory {
  public:
    TcpTlsFactory(TlsRole role, const TlsConfig &config);
    ~TcpTlsFactory();

    virtual bool initialize() override;
    virtual TcpConnector* createConnector(event::Loop *wp_loop) override;
    virtual TcpAcceptor*  createAcceptor(event::Loop *wp_loop) override;

  private:
    TlsRole  role_;
    TlsConfig tls_config_;
    SSL_CTX  *ssl_ctx_ = nullptr;
};

}
}
#endif //TBOX_NETWORK_TCP_TLS_FACTORY_H_20260616
