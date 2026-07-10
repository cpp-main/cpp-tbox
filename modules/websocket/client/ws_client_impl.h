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
#include <tbox/event/timer_event.h>
#include <tbox/base/defines.h>
#include <tbox/network/sockaddr.h>
#include <tbox/network/tcp_connector.h>
#include <tbox/network/tcp_connection.h>

#include "ws_client.h"
#include "../ws_frame.h"
#include "../ws_frame_parser.h"
#include "../ws_compressor.h"

namespace tbox {
namespace websocket {
namespace client {

//! WsClient::Impl 实现完整的 WebSocket 客户端
//! 流程：TcpConnector 建立 TCP → 发送 HTTP Upgrade → 验证 101 → 帧通信
//! 发送大数据时先压缩再分片发送，避免单帧过大
class WsClient::Impl {
  public:
    //! 默认分片发送的最大帧 payload 大小（64KB）
    //! 选择 65535 是因为：不超过 16-bit payload length 编码范围，避免 64-bit 编码开销
    static constexpr size_t kDefaultFragmentSize = 65535;
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
    void setTextMessageCallback(const WsClient::TextMessageCallback &cb)   { text_message_cb_ = cb; }
    void setBinaryMessageCallback(const WsClient::BinaryMessageCallback &cb) { binary_message_cb_ = cb; }
    void setErrorCallback(const WsClient::ErrorCallback &cb)            { error_cb_ = cb; }
    void setAutoReconnect(bool enable) { reconnect_enabled_ = enable; }
    void setReconnectDelayCalcFunc(const WsClient::ReconnectDelayCalc &func);
    void setCompressionPrefer(bool enable) { prefer_compression_ = enable; }
    void setFragmentSize(size_t size) { fragment_size_ = size; }
    void setPingInterval(int seconds) { ping_interval_ = seconds; }
    void setPingTimeout(int seconds) { ping_timeout_ = seconds; }

  public:
    bool send(const std::string &text);
    bool send(const char *str);
    bool send(const void *data, size_t len);
    bool send(const std::vector<uint8_t> &data);
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

    //! 分片发送 payload（客户端，掩码，内部使用）
    //! opcode: 首帧 opcode（kText 或 kBinary）
    //! payload/payload_len: 完整的 payload 数据（可能为压缩后数据）
    //! is_compressed: 是否为压缩数据（首帧设置 rsv1=true）
    bool sendFragmented(WsFrame::OpCode opcode, const void *payload, size_t payload_len, bool is_compressed);

    //! 统一发送数据（内部使用）
    //! opcode: kText 或 kBinary
    //! data_ptr/data_len: 原始数据指针与长度
    //! 流程：前置检查 → 压缩(如需要) → sendFragmented
    bool sendData(WsFrame::OpCode opcode, const void *data_ptr, size_t data_len);

    //! 将完整消息交付给业务层（opcode 为 kText 或 kBinary）
    void deliverMessage(WsFrame::OpCode opcode, std::string &data);

    //! 出错处理
    void onError();

    //! Ping/Pong 心跳定时器回调
    void onPingTimerFired();
    void onPongTimeoutFired();

  private:
    WsClient *wp_parent_;
    event::Loop *wp_loop_;

    network::TcpConnector *sp_connector_ = nullptr;
    network::TcpConnection *sp_tcp_conn_ = nullptr;

    network::SockAddr server_addr_;
    std::string url_path_;

    //! 握手阶段暂存：Sec-WebSocket-Key（用于验证 Accept）
    std::string sec_ws_key_;

    //! 帧解析器
    WsFrameParser frame_parser_;

    //! 压缩相关
    bool prefer_compression_ = false;
    WsCompressionConfig compression_config_;  //! 握手成功后确认的压缩配置
    WsCompressor compressor_;

    //! 分片发送的最大帧 payload 大小（可配置，默认 kDefaultFragmentSize）
    size_t fragment_size_ = WsClient::kDefaultFragmentSize;

    //! Ping/Pong 心跳参数
    int ping_interval_ = 0;
    int ping_timeout_ = 0;
    event::TimerEvent *sp_ping_timer_ = nullptr;
    event::TimerEvent *sp_pong_timer_ = nullptr;
    bool is_pong_pending_ = false;

    WsClient::State state_ = WsClient::State::kNone;

    WsClient::ConnectedCallback     connected_cb_;
    WsClient::DisconnectedCallback  disconnected_cb_;
    WsClient::TextMessageCallback   text_message_cb_;
    WsClient::BinaryMessageCallback binary_message_cb_;
    WsClient::ErrorCallback         error_cb_;

    bool is_closing_ = false;
    bool reconnect_enabled_ = true;
    int cb_level_ = 0;

    //! 分片组装相关
    //! 只有接收完整数据帧（fin=true）并进行解压后，才回调业务层
    bool is_fragmenting_ = false;             //!< 是否正在接收分片消息
    WsFrame::OpCode fragment_opcode_;         //!< 分片消息的原始 opcode（kText 或 kBinary）
    bool fragment_need_decompress_ = false;   //!< 分片消息是否需要解压
    std::string fragment_buffer_;             //!< 分片数据的缓存区
};

}
}
}

#endif //TBOX_WS_CLIENT_IMPL_H_20260615
