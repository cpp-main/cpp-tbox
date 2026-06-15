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
#ifndef TBOX_WS_SERVER_H_20260612
#define TBOX_WS_SERVER_H_20260612

#include <tbox/event/loop.h>
#include <tbox/base/cabinet_token.h>
#include <tbox/base/defines.h>
#include <tbox/network/sockaddr.h>

#include <tbox/http/request.h>

#include "ws_frame.h"

namespace tbox {
namespace http {
namespace server {
class Server;
}
}

namespace websocket {

//! WebSocket 服务器
//! 基于 HTTP 服务器运行，本身即为 HTTP 中间件
//! 支持指定 URL 路径（精确匹配），实现多个 WebSocket 服务挂载于同一 HTTP 服务器
//! 升级后接管 TcpConnection，提供 WebSocket 通信功能
//! 通过 Cabinet 管理 WsConnection 生命期，用户通过 ConnToken 操作连接
class WsServer {
  public:
    using ConnToken = cabinet::Token;

    explicit WsServer(event::Loop *wp_loop);
    ~WsServer();

    NONCOPYABLE(WsServer);
    IMMOVABLE(WsServer);

  public:
    //! 初始化：关联到 HTTP 服务器
    //! url_path 为 WebSocket 服务挂载的 URL 路径（精确匹配）
    //! 空字符串表示匹配所有 WebSocket 升级请求
    bool initialize(http::server::Server *http_server, const std::string &url_path = "");
    bool start();
    void stop();
    void cleanup();

    enum class State { kNone, kInited, kRunning };
    State state() const;

  public:
    //! 设置回调（所有回调均使用 ConnToken，不暴露 WsConnection 指针）
    using ConnectedCallback    = std::function<void(const ConnToken &)>;
    using DisconnectedCallback = std::function<void(const ConnToken &)>;
    using MessageCallback      = std::function<void(const ConnToken &, const WsFrame&)>;
    using ErrorCallback        = std::function<void(const ConnToken &)>;

    void setConnectedCallback(const ConnectedCallback &cb);
    void setDisconnectedCallback(const DisconnectedCallback &cb);
    void setMessageCallback(const MessageCallback &cb);
    void setErrorCallback(const ErrorCallback &cb);

  public:
    //! 向指定客户端发送文本数据
    bool send(const ConnToken &client, const std::string &text);
    //! 向指定客户端发送二进制数据
    bool send(const ConnToken &client, const void *data, size_t len);
    //! 向指定客户端发送二进制数据（vector 版本）
    bool sendBinary(const ConnToken &client, const std::vector<uint8_t> &data);

    //! 关闭指定客户端连接（发送 Close 帧）
    bool close(const ConnToken &client, uint16_t code = 1000, const std::string &reason = "");

    //! 发送 Ping 帧
    bool ping(const ConnToken &client, const std::string &data = "");
    //! 发送 Pong 帧
    bool pong(const ConnToken &client, const std::string &data = "");

    //! 检查客户端连接是否有效
    bool isClientValid(const ConnToken &client) const;
    //! 获取客户端地址
    network::SockAddr peerAddr(const ConnToken &client) const;

  public:
    //! 检查请求是否为有效的 WebSocket 升级请求
    static bool IsWsUpgradeRequest(const http::Request &req);

    //! 计算 Sec-WebSocket-Accept 响应值
    static std::string ComputeWsAcceptKey(const std::string &sec_ws_key);

  private:
    class Impl;
    Impl *impl_;
};

}
}

#endif //TBOX_WS_SERVER_H_20260612
