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
#ifndef TBOX_WS_CLIENT_H_20260615
#define TBOX_WS_CLIENT_H_20260615

#include <tbox/event/loop.h>
#include <tbox/network/sockaddr.h>
#include <tbox/network/tls_config.h>
#include <tbox/base/defines.h>

#include "../ws_frame.h"

namespace tbox {
namespace websocket {
namespace client {

//! WebSocket 客户端
//! 通过 TcpConnector 建立 TCP 连接，发送 HTTP Upgrade 握手
//! 握手成功后进入 WebSocket 帧通信模式（客户端帧必须掩码）
//! 断连后支持自动重连（默认开启），重连延迟策略委托给 TcpConnector
class WsClient {
  public:
    explicit WsClient(event::Loop *wp_loop);
    ~WsClient();

    NONCOPYABLE(WsClient);
    IMMOVABLE(WsClient);

  public:
    //! 初始化：设置目标服务器地址与 URL 路径
    //! server_addr 为服务器地址（如 SockAddr::FromString("127.0.0.1:8080")）
    //! url_path 为 WebSocket 路径（如 "/ws/chat"）
    bool initialize(const network::SockAddr &server_addr, const std::string &url_path = "/");

    bool start();
    void stop();
    void cleanup();

    enum class State { kNone, kInited, kConnecting, kHandshaking, kConnected };
    State state() const;

  public:
    //! 设置回调
    using ConnectedCallback    = std::function<void()>;
    using DisconnectedCallback = std::function<void()>;
    using MessageCallback      = std::function<void(const WsFrame&)>;
    using ErrorCallback        = std::function<void()>;

    //! 重连延迟策略（与 TcpClient 一致，委托给 TcpConnector）
    using ReconnectDelayCalc   = std::function<int(int)>;

    void setConnectedCallback(const ConnectedCallback &cb);
    void setDisconnectedCallback(const DisconnectedCallback &cb);
    void setMessageCallback(const MessageCallback &cb);
    void setErrorCallback(const ErrorCallback &cb);

    //! 是否启用自动重连（默认开启）
    void setAutoReconnect(bool enable);
    //! 设置自定义重连延迟策略（委托给底层 TcpConnector）
    void setReconnectDelayCalcFunc(const ReconnectDelayCalc &func);

    //! 设置 TLS 配置（必须在 initialize() 之前调用）
    //! 需要 network_tls 模块支持，未链接时调用无效
    void setTlsConfig(const network::TlsConfig &config);

  public:
    //! 发送文本帧
    bool send(const std::string &text);
    //! 发送二进制帧
    bool send(const void *data, size_t len);
    bool sendBinary(const std::vector<uint8_t> &data);

    //! 发送关闭帧并关闭连接
    bool close(uint16_t code = 1000, const std::string &reason = "");

    //! 发送 Ping 帧
    bool ping(const std::string &data = "");
    //! 发送 Pong 帧
    bool pong(const std::string &data = "");

    //! 连接是否已失效
    bool isExpired() const;

    //! 获取服务器地址
    network::SockAddr peerAddr() const;

    //! 设置/获取上下文数据
    using ContextDeleter = std::function<void(void*)>;
    void  setContext(void *context, ContextDeleter &&deleter = nullptr);
    void* getContext() const;

  private:
    class Impl;
    Impl *impl_;
};

}
}
}

#endif //TBOX_WS_CLIENT_H_20260615
