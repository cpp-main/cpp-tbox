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
#include "tcp_tls_connector.h"
#include "tcp_tls_connection.h"

#include <openssl/err.h>
#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>
#include <tbox/event/fd_event.h>

#undef  MODULE_ID
#define MODULE_ID "tbox.tcp_tls"

namespace tbox {
namespace network {

TcpTlsConnector::TcpTlsConnector(event::Loop *wp_loop, SSL_CTX *ssl_ctx, const TlsConfig &tls_config) :
    TcpConnector(wp_loop),
    ssl_ctx_(ssl_ctx),
    tls_config_(tls_config)
{ }

TcpTlsConnector::~TcpTlsConnector()
{
    //! 如果握手还在进行中，需要清理
    if (sp_handshake_ev_ != nullptr) {
        sp_handshake_ev_->disable();
        CHECK_DELETE_RESET_OBJ(sp_handshake_ev_);
    }

    if (handshake_ssl_ != nullptr) {
        SSL_free(handshake_ssl_);
        handshake_ssl_ = nullptr;
    }

    handshake_fd_.close();
}

TcpConnection* TcpTlsConnector::createConnection(event::Loop *, SocketFd, const SockAddr &)
{
    //! 注意：此方法只在 SSL 握手成功后由 onSslHandshakeSuccess 调用
    //! 此时 handshake_ssl_ 已经是完全建立的 SSL 连接
    //! 但 handshake_ssl_ 已在 onSslHandshakeSuccess 中置 nullptr，需要通过参数传入
    //! 实际上不直接使用此方法，而是在 onSslHandshakeSuccess 中直接创建 TcpTlsConnection
    return nullptr;  //! 不直接使用此方法
}

void TcpTlsConnector::onTcpConnected(SocketFd fd, const SockAddr &peer_addr)
{
    //! TCP 连接成功后，不立即创建 Connection，而是开始 SSL 握手
    startSslHandshake(fd, peer_addr);
}

void TcpTlsConnector::startSslHandshake(SocketFd fd, const SockAddr &peer_addr)
{
    //! 创建 SSL 对象
    handshake_ssl_ = SSL_new(ssl_ctx_);
    if (handshake_ssl_ == nullptr) {
        LogErr("SSL_new fail");
        fd.close();
        onConnectFail();
        return;
    }

    //! 设置 SNI (Server Name Indication)
    if (!tls_config_.hostname.empty()) {
        SSL_set_tlsext_host_name(handshake_ssl_, tls_config_.hostname.c_str());
    }

    //! 将 fd 绑定到 SSL
    SSL_set_fd(handshake_ssl_, fd.get());

    //! 保存握手需要的参数
    handshake_fd_ = fd; //! 不要使用 handshake_fd_.swap(fd) 否则会有问题
    handshake_peer_addr_ = peer_addr;

    //! 开始 SSL_connect
    ERR_clear_error();
    int ret = SSL_connect(handshake_ssl_);

    if (ret == 1) {
        //! SSL 握手立即完成（罕见，通常需要多次 WANT_READ/WANT_WRITE）
        onSslHandshakeSuccess();
        return;
    }

    int ssl_error = SSL_get_error(handshake_ssl_, ret);
    if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
        //! 正常的异步握手过程，需要等待 fd 事件
        short events = (ssl_error == SSL_ERROR_WANT_READ) ? event::FdEvent::kReadEvent : event::FdEvent::kWriteEvent;

        CHECK_DELETE_RESET_OBJ(sp_handshake_ev_);
        sp_handshake_ev_ = wp_loop_->newFdEvent("TcpTlsConnector::sp_handshake_ev_");
        sp_handshake_ev_->initialize(fd.get(), events, event::Event::Mode::kOneshot);
        sp_handshake_ev_->setCallback(std::bind(&TcpTlsConnector::onSslHandshakeEvent, this, std::placeholders::_1));
        sp_handshake_ev_->enable();

        LogDbg("SSL handshake in progress, waiting for %s", (ssl_error == SSL_ERROR_WANT_READ) ? "READ" : "WRITE");
    } else {
        //! SSL 握手失败
        LogErr("SSL_connect fail, error:%d", ssl_error);
        SSL_free(handshake_ssl_);
        handshake_ssl_ = nullptr;
        handshake_fd_.close();
        onConnectFail();
    }
}

void TcpTlsConnector::onSslHandshakeEvent(short)
{
    //! 继续 SSL_connect
    ERR_clear_error();
    int ret = SSL_connect(handshake_ssl_);

    if (ret == 1) {
        //! 握手成功
        onSslHandshakeSuccess();
        return;
    }

    int ssl_error = SSL_get_error(handshake_ssl_, ret);
    if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
        //! 需要继续等待
        //! kOneshot 事件触发后已自动 disable，可以直接 reinitialize，无需重新创建 FdEvent
        //! 这避免了在回调中删除 FdEvent 导致的 assert 失败
        short next_events = (ssl_error == SSL_ERROR_WANT_READ) ? event::FdEvent::kReadEvent : event::FdEvent::kWriteEvent;
        sp_handshake_ev_->initialize(handshake_fd_.get(), next_events, event::Event::Mode::kOneshot);
        sp_handshake_ev_->enable();

        LogDbg("SSL handshake continue, waiting for %s", (ssl_error == SSL_ERROR_WANT_READ) ? "READ" : "WRITE");
    } else {
        //! 握手失败
        LogErr("SSL handshake fail, error:%d", ssl_error);
        onSslHandshakeFail();
    }
}

void TcpTlsConnector::onSslHandshakeSuccess()
{
    RECORD_SCOPE();

    //! 清理握手相关的 FdEvent
    //! 不能在回调中直接删除 FdEvent（cb_level_ > 0），需要延后删除
    if (sp_handshake_ev_ != nullptr) {
        sp_handshake_ev_->disable();
        event::FdEvent *tmp = nullptr;
        std::swap(tmp, sp_handshake_ev_);
        wp_loop_->runNext(
            [tmp] { delete tmp; },
            "TcpTlsConnector::onSslHandshakeSuccess, delete ev"
        );
    }

    //! 将 SSL 和 fd 从握手状态转移到 TcpTlsConnection
    SSL *ssl = handshake_ssl_;
    handshake_ssl_ = nullptr;  //! 防止析构时重复释放
    SocketFd fd = handshake_fd_;
    handshake_fd_.reset();     //! 防止析构时重复关闭
    SockAddr peer_addr = handshake_peer_addr_;

    LogInfo("TLS handshake to %s success", peer_addr.toString().c_str());

    //! 创建 TcpTlsConnection 并触发回调
    if (connected_cb_) {
        auto sp_conn = new TcpTlsConnection(wp_loop_, fd, peer_addr, ssl);
        sp_conn->enable();
        ++cb_level_;
        connected_cb_(sp_conn);
        --cb_level_;
    } else {
        LogWarn("connected callback is not set");
        //! 没有回调，需要释放 SSL 和关闭 fd
        SSL_free(ssl);
        fd.close();
    }
}

void TcpTlsConnector::onSslHandshakeFail()
{
    //! 清理握手相关的资源
    //! 不能在回调中直接删除 FdEvent（cb_level_ > 0），需要延后删除
    if (sp_handshake_ev_ != nullptr) {
        sp_handshake_ev_->disable();
        event::FdEvent *tmp = nullptr;
        std::swap(tmp, sp_handshake_ev_);
        wp_loop_->runNext(
            [tmp] { delete tmp; },
            "TcpTlsConnector::onSslHandshakeFail, delete ev"
        );
    }

    SSL_free(handshake_ssl_);
    handshake_ssl_ = nullptr;
    handshake_fd_.close();

    //! SSL 握手失败视为连接失败，触发重连逻辑
    LogNotice("TLS handshake fail, treat as connection fail");
    onConnectFail();
}

}
}
