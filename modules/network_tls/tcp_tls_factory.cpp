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
#include "tcp_tls_factory.h"
#include "tcp_tls_connector.h"
#include "tcp_tls_acceptor.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/scope_exit.hpp>

#undef  MODULE_ID
#define MODULE_ID "tbox.tcp_tls"

namespace tbox {
namespace network {

namespace {
//! 加载证书与私钥到 SSL_CTX
//! 成功返回 true，失败返回 false
bool LoadCertAndKey(SSL_CTX *ctx, const std::string &cert_file, const std::string &key_file)
{
    if (SSL_CTX_use_certificate_file(ctx, cert_file.c_str(), SSL_FILETYPE_PEM) != 1) {
        LogErr("SSL_CTX_use_certificate_file fail, file:%s", cert_file.c_str());
        ERR_print_errors_fp(stderr);
        return false;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, key_file.c_str(), SSL_FILETYPE_PEM) != 1) {
        LogErr("SSL_CTX_use_PrivateKey_file fail, file:%s", key_file.c_str());
        ERR_print_errors_fp(stderr);
        return false;
    }
    if (SSL_CTX_check_private_key(ctx) != 1) {
        LogErr("SSL_CTX_check_private_key fail");
        return false;
    }
    return true;
}

//! 加载 CA 证书到 SSL_CTX
//! 成功返回 true，失败返回 false
bool LoadCaCert(SSL_CTX *ctx, const TlsConfig &tls_config)
{
    const char *ca_file = tls_config.ca_file.empty() ? nullptr : tls_config.ca_file.c_str();
    const char *ca_path = tls_config.ca_path.empty() ? nullptr : tls_config.ca_path.c_str();

    if (SSL_CTX_load_verify_locations(ctx, ca_file, ca_path) != 1) {
        LogErr("SSL_CTX_load_verify_locations fail, ca_file:%s, ca_path:%s",
               tls_config.ca_file.c_str(), tls_config.ca_path.c_str());
        ERR_print_errors_fp(stderr);
        return false;
    }
    return true;
}

SSL_CTX* CreateClientSslCtx(const TlsConfig &tls_config)
{
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (ctx == nullptr) {
        LogErr("SSL_CTX_new(TLS_client_method) fail");
        return nullptr;
    }

    ScopeExitActionGuard guard([ctx] { SSL_CTX_free(ctx); });

    //! 设置最低 TLS 版本为 1.2
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);

    //! 加载 CA 证书（用于验证 server）
    if (!tls_config.ca_file.empty() || !tls_config.ca_path.empty()) {
        if (!LoadCaCert(ctx, tls_config))
            return nullptr;

        if (tls_config.verify_peer)
            SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
    } else if (tls_config.verify_peer) {
        //! 使用系统默认 CA 证书
        if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
            LogErr("SSL_CTX_set_default_verify_paths fail");
            return nullptr;
        }
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
    } else {
        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);
    }

    //! 加载本端证书和密钥（可选，用于双向 TLS）
    if (!tls_config.cert_file.empty() && !tls_config.key_file.empty()) {
        if (!LoadCertAndKey(ctx, tls_config.cert_file, tls_config.key_file)) {
            return nullptr;
        }
    }

    guard.cancel();
    return ctx;
}

SSL_CTX* CreateServerSslCtx(const TlsConfig &tls_config)
{
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (ctx == nullptr) {
        LogErr("SSL_CTX_new(TLS_server_method) fail");
        return nullptr;
    }

    ScopeExitActionGuard guard([ctx] { SSL_CTX_free(ctx); });

    //! 设置最低 TLS 版本为 1.2
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);

    //! 加载本端证书和密钥（Server 必须设置）
    if (!tls_config.cert_file.empty() && !tls_config.key_file.empty()) {
        if (!LoadCertAndKey(ctx, tls_config.cert_file, tls_config.key_file))
            return nullptr;

    } else {
        LogErr("server cert_file and key_file must be set");
        return nullptr;
    }

    //! 加载 CA 证书（可选，用于验证 client - 双向 TLS）
    if (!tls_config.ca_file.empty() || !tls_config.ca_path.empty()) {
        if (!LoadCaCert(ctx, tls_config))
            return nullptr;

        if (tls_config.verify_peer) {
            SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
            SSL_CTX_set_verify_depth(ctx, tls_config.verify_depth);
        }
    }

    guard.cancel();
    return ctx;
}
}

///////////////////////////////////////////////////////

TcpTlsFactory::TcpTlsFactory(TlsRole role, const TlsConfig &config)
  : role_(role)
  , tls_config_(config)
{ }

TcpTlsFactory::~TcpTlsFactory()
{
    if (ssl_ctx_ != nullptr)
        SSL_CTX_free(ssl_ctx_);
}

bool TcpTlsFactory::initialize()
{
    if (role_ == TlsRole::kClient) {
        ssl_ctx_ = CreateClientSslCtx(tls_config_);
    } else if (role_ == TlsRole::kServer) {
        ssl_ctx_ = CreateServerSslCtx(tls_config_);
    }
    return ssl_ctx_ != nullptr;
}

TcpConnector* TcpTlsFactory::createConnector(event::Loop *wp_loop)
{
    TBOX_ASSERT(role_ == TlsRole::kClient);
    TBOX_ASSERT(ssl_ctx_ != nullptr);
    return new TcpTlsConnector(wp_loop, ssl_ctx_, tls_config_);
}

TcpAcceptor* TcpTlsFactory::createAcceptor(event::Loop *wp_loop)
{
    TBOX_ASSERT(role_ == TlsRole::kServer);
    TBOX_ASSERT(ssl_ctx_ != nullptr);
    return new TcpTlsAcceptor(wp_loop, ssl_ctx_, tls_config_);
}

}
}
