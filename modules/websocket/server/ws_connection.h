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
#ifndef TBOX_WS_CONNECTION_H_20260612
#define TBOX_WS_CONNECTION_H_20260612

#include <vector>
#include <functional>

#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/network/tcp_connection.h>

#include "../ws_frame.h"
#include "../ws_frame_parser.h"
#include "../ws_compressor.h"

namespace tbox {
namespace websocket {
namespace server {

//! WebSocket 连接
//! 包装从 HTTP 升级后分离出来的 TcpConnection，解析/构建 WebSocket 帧
//! 生命期由 WsServer 通过 Cabinet 管理，用户通过 ConnToken 访问
//! 支持分片消息的完整接收：缓存分片数据，接收完整后再解压并回调
//! 发送大数据时先压缩再分片发送，避免单帧过大
class WsConnection {
  public:
    //! 默认分片发送的最大帧 payload 大小（64KB）
    //! 选择 65535 是因为：不超过 16-bit payload length 编码范围，避免 64-bit 编码开销
    static constexpr size_t kDefaultFragmentSize = 65535;

    //! 内部回调：WsServer::Impl 绑定 ConnToken，不传递 WsConnection*
    using CloseCallback        = std::function<void()>;
    using TextMessageCallback  = std::function<void(std::string &&)>;
    using BinaryMessageCallback = std::function<void(std::vector<uint8_t> &&)>;
    using ErrorCallback        = std::function<void()>;
    using PingCallback         = std::function<void(const std::string&)>;
    using PongCallback         = std::function<void(const std::string&)>;
    using SendCompleteCallback = std::function<void()>;

    ~WsConnection();

    NONCOPYABLE(WsConnection);
    IMMOVABLE(WsConnection);

  public:
    //! 设置回调（由 WsServer::Impl 调用，绑定 ConnToken）
    void setCloseCallback(const CloseCallback &cb)              { close_cb_ = cb; }
    void setTextMessageCallback(const TextMessageCallback &cb)  { text_message_cb_ = cb; }
    void setBinaryMessageCallback(const BinaryMessageCallback &cb) { binary_message_cb_ = cb; }
    void setErrorCallback(const ErrorCallback &cb)              { error_cb_ = cb; }
    void setPingCallback(const PingCallback &cb)                { ping_cb_ = cb; }
    void setPongCallback(const PongCallback &cb)                { pong_cb_ = cb; }
    void setSendCompleteCallback(const SendCompleteCallback &cb) { send_complete_cb_ = cb; }

  public:
    //! 发送文本帧
    bool send(const std::string &text);
    //! 发送文本帧（const char* 版本，方便直接传字符串字面量）
    bool send(const char *str);
    //! 发送二进制帧
    bool send(const void *data, size_t len);
    bool send(const std::vector<uint8_t> &data);

    //! 发送关闭帧并关闭连接
    bool close(uint16_t code = 1000, const std::string &reason = "");

    //! 发送 Ping 帧
    bool ping(const std::string &data = "");
    //! 发送 Pong 帧
    bool pong(const std::string &data = "");

    //! 获取客户端地址
    network::SockAddr peerAddr() const;

    //! 获取客户端连接的 URL 路径
    std::string getUrl() const;

    //! 连接是否已失效
    bool isExpired() const;

    //! 设置/获取上下文数据（直接委托给底层 TcpConnection）
    using ContextDeleter = network::TcpConnection::ContextDeleter;
    void  setContext(void *context, ContextDeleter &&deleter = nullptr);
    void* getContext() const;

    //! 设置/获取分片大小（仅影响发送，接收时自动组装）
    void setFragmentSize(size_t size) { fragment_size_ = size; }
    size_t fragmentSize() const { return fragment_size_; }

    //! 设置 Ping/Pong 心跳参数
    void setPingInterval(int seconds);
    void setPingTimeout(int seconds);

  private:
    //! 仅由 WsServer 创建（生命期由 Cabinet 管理）
    //! compress_config 为握手时协商的压缩配置
    //! fragment_size 为分片发送的最大帧 payload 大小
    //! ping_interval/ping_timeout 为心跳参数（0=不自动 Ping/不检测超时）
    WsConnection(event::Loop *wp_loop, network::TcpConnection *tcp_conn, const std::string &url,
                 const WsCompressionConfig &compress_config, size_t fragment_size,
                 int ping_interval, int ping_timeout);

    void onTcpReceived(network::Buffer &buff);
    void onTcpDisconnected();
    void onTcpSendCompleted();

    //! 发送 WebSocket 帧（内部使用）
    bool sendFrame(WsFrame::OpCode opcode, bool fin, const void *payload, size_t payload_len);

    //! 分片发送 payload（内部使用）
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

    //! Ping/Pong 心跳定时器回调
    void onPingTimerFired();
    void onPongTimeoutFired();

  private:
    event::Loop *wp_loop_;
    network::TcpConnection *sp_tcp_conn_;
    std::string url_;

    WsFrameParser frame_parser_;

    //! 压缩相关
    WsCompressionConfig compression_config_;
    WsCompressor compressor_;

    //! 分片发送的最大帧 payload 大小（可配置，默认 kDefaultFragmentSize）
    size_t fragment_size_ = kDefaultFragmentSize;

    CloseCallback        close_cb_;
    TextMessageCallback  text_message_cb_;
    BinaryMessageCallback binary_message_cb_;
    ErrorCallback        error_cb_;
    PingCallback         ping_cb_;
    PongCallback         pong_cb_;
    SendCompleteCallback send_complete_cb_;

    bool is_closing_ = false;

    //! Ping/Pong 心跳相关
    int ping_interval_ = 0;         //! Ping 发送间隔（秒，0=不自动 Ping）
    int ping_timeout_ = 0;          //! Pong 超时时间（秒，0=不检测超时）
    event::TimerEvent *sp_ping_timer_ = nullptr;  //! Ping 定时器（周期触发）
    event::TimerEvent *sp_pong_timer_ = nullptr;  //! Pong 超时定时器（单次触发）
    bool is_pong_pending_ = false;   //! 发送 Ping 后是否在等待 Pong 回复

    //! 分片组装相关
    //! 只有接收完整数据帧（fin=true）并进行解压后，才回调业务层
    bool is_fragmenting_ = false;             //!< 是否正在接收分片消息
    WsFrame::OpCode fragment_opcode_;         //!< 分片消息的原始 opcode（kText 或 kBinary）
    bool fragment_need_decompress_ = false;   //!< 分片消息是否需要解压
    std::string fragment_buffer_;             //!< 分片数据的缓存区

    int cb_level_ = 0;

    friend class WsServer;
};

}
}
}

#endif //TBOX_WS_CONNECTION_H_20260612
