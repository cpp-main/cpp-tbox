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
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_WS_CLIENT_IMPL_H_20260615
#define TBOX_WS_CLIENT_IMPL_H_20260615

#include <tbox/event/loop.h>
#include <tbox/base/defines.h>
#include <tbox/network/sockaddr.h>
#include <tbox/network/tcp_connector.h>
#include <tbox/network/tcp_factory.h>
#include <tbox/network/tls_factory_entry.h>
#include <tbox/network/tcp_connection.h>

#include "ws_client.h"
#include "../ws_frame.h"
#include "../ws_frame_parser.h"

namespace tbox {
namespace websocket {
namespace client {

//! WsClient::Impl 实现完整的 WebSocket 客户端
//! 流程：TcpConnector 建立 TCP → 发送 HTTP Upgrade → 验证 101 → 帧通信
class WsClient::Impl {
  public:
    Impl(WsClient *wp_parent, event::Loop *wp_loop);
    ~Impl();

  public:
    bool initialize(const network::SockAddr &server_addr, const std::string &url_path);
    bool start();
    void stop();
    void cleanup();

    WsClient::State state() const { return state_; }

  public:
    void setConnectedCallback(const WsClient::ConnectedCallback &cb)    { connected_cb_ = cb; }
    void setDisconnectedCallback(const WsClient::DisconnectedCallback &cb) { disconnected_cb_ = cb; }
    void setMessageCallback(const WsClient::MessageCallback &cb)        { message_cb_ = cb; }
    void setErrorCallback(const WsClient::ErrorCallback &cb)            { error_cb_ = cb; }
    void setAutoReconnect(bool enable) { reconnect_enabled_ = enable; }
    void setReconnectDelayCalcFunc(const WsClient::ReconnectDelayCalc &func);
    void setTlsConfig(const network::TlsConfig &config);

  public:
    bool send(const std::string &text);
    bool send(const void *data, size_t len);
    bool sendBinary(const std::vector<uint8_t> &data);
    bool close(uint16_t code, const std::string &reason);
    bool ping(const std::string &data);
    bool pong(const std::string &data);
    bool isExpired() const;
    network::SockAddr peerAddr() const;

    using ContextDeleter = network::TcpConnection::ContextDeleter;
    void  setContext(void *context, ContextDeleter &&deleter = nullptr);
    void* getContext() const;

  private:
    //! TCP 连接建立成功
    void onTcpConnected(network::TcpConnection *tcp_conn);

    //! TCP 连接断开
    void onTcpDisconnected();

    //! TCP 收到数据（握手阶段与帧通信阶段共用）
    void onTcpReceived(network::Buffer &buff);

    //! === 握手阶段 ===

    //! 构造并发送 HTTP Upgrade 握手请求
    void sendHandshakeRequest();

    //! 解析服务器握手响应，验证 101 + Sec-WebSocket-Accept
    bool parseHandshakeResponse(network::Buffer &buff);

    //! 握手成功，进入帧通信模式
    void onHandshakeSuccess();

    //! 握手失败
    void onHandshakeFail();

    //! === 帧通信阶段 ===

    //! 解析 WebSocket 帧
    void onWsFrameReceived(network::Buffer &buff);

    //! 发送 WebSocket 帧（客户端，掩码）
    bool sendMaskedFrame(WsFrame::OpCode opcode, bool fin, const void *payload, size_t payload_len);

    //! 出错处理
    void onError();

  private:
    WsClient *wp_parent_;
    event::Loop *wp_loop_;

    network::TcpFactory *sp_factory_ = nullptr;
    network::TcpConnector *sp_connector_ = nullptr;
    network::TcpConnection *sp_tcp_conn_ = nullptr;

    network::SockAddr server_addr_;
    std::string url_path_;

    //! 握手阶段暂存：Sec-WebSocket-Key（用于验证 Accept）
    std::string sec_ws_key_;

    //! 帧解析器
    WsFrameParser frame_parser_;

    WsClient::State state_ = WsClient::State::kNone;

    WsClient::ConnectedCallback    connected_cb_;
    WsClient::DisconnectedCallback disconnected_cb_;
    WsClient::MessageCallback      message_cb_;
    WsClient::ErrorCallback        error_cb_;

    bool is_closing_ = false;
    bool reconnect_enabled_ = true;
    int cb_level_ = 0;
};

}
}
}

#endif //TBOX_WS_CLIENT_IMPL_H_20260615
