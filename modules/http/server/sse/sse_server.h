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
#ifndef TBOX_HTTP_SSE_SERVER_H_20260616
#define TBOX_HTTP_SSE_SERVER_H_20260616

#include <chrono>
#include <functional>

#include <tbox/event/forward.h>
#include <tbox/base/cabinet_token.h>
#include <tbox/base/defines.h>
#include <tbox/network/sockaddr.h>

#include "sse_event.h"

namespace tbox {
namespace http {

namespace server {
class Server;
}

namespace sse {

//! SSE 服务器（Server-Sent Events, W3C/WHATWG 规范）
//! 基于 HTTP 服务器运行，本身即为 HTTP 中间件
//! 检测 SSE 请求（Accept: text/event-stream），设置 200 响应头
//! 通过 upgrade_cb 机制接管 TcpConnection，提供 SSE 事件推送功能
//! SSE 是单向推送协议（服务端→客户端），无 MessageCallback
//! 通过 Cabinet 管理 SseConnection 生命期，用户通过 ConnToken 操作连接
class SseServer {
  public:
    using ConnToken = cabinet::Token;

    explicit SseServer(event::Loop *wp_loop);
    ~SseServer();

    NONCOPYABLE(SseServer);
    IMMOVABLE(SseServer);

  public:
    //! 初始化：关联到 HTTP 服务器
    //! URL 路径匹配规则：
    //! - url_path 以 '/' 结尾：前缀匹配，如 "/sse/" 匹配 "/sse/aa"、" /sse/bb/cc"
    //! - url_path 不以 '/' 结尾：全量匹配，如 "/sse" 仅匹配 "/sse"
    //! - url_path 为空字符串：匹配所有 SSE 请求
    bool initialize(http::server::Server *http_server, const std::string &url_path = "");
    bool start();
    void stop();
    void cleanup();

    enum class State { kNone, kInited, kRunning };
    State state() const;

  public:
    //! 设置回调（SSE 是单向推送，无 MessageCallback）
    using ConnectedCallback    = std::function<void(const ConnToken &)>;
    using DisconnectedCallback = std::function<void(const ConnToken &)>;

    void setConnectedCallback(const ConnectedCallback &cb);
    void setDisconnectedCallback(const DisconnectedCallback &cb);

  public:
    //! 向指定客户端发送数据（简单文本，event 类型默认 "message"）
    //! 格式："data: <text>\n\n"
    bool send(const ConnToken &client, const std::string &data);

    //! 向指定客户端发送完整 SSE 事件
    bool send(const ConnToken &client, const SseEvent &event);

    //! 向所有客户端广播数据
    bool sendToAll(const std::string &data);

    //! 向所有客户端广播事件
    bool sendToAll(const SseEvent &event);

    //! 关闭指定客户端连接
    bool close(const ConnToken &client);

    //! 发送心跳注释行（保持连接活跃）
    //! 格式：": <comment>\n\n"，浏览器 EventSource 忽略以 ":" 开头的行
    bool sendHeartbeat(const ConnToken &client, const std::string &comment = "keep-alive");

    //! 设置自动心跳间隔（默认 0 = 禁用）
    //! 启用后，定时器每 interval 毫秒向所有连接发送 ": keep-alive\n\n"
    void setHeartbeatInterval(std::chrono::milliseconds interval);

    //! 检查客户端连接是否有效
    bool isClientValid(const ConnToken &client) const;

    //! 获取客户端地址（含 IP 与端口）
    network::SockAddr peerAddr(const ConnToken &client) const;

    //! 获取客户端请求的 Last-Event-ID（浏览器重连时携带）
    //! 用于实现断线续传：服务端可根据此值从断点继续推送
    std::string getLastEventId(const ConnToken &client) const;

    //! 获取客户端连接的 URL 路径
    std::string getUrl(const ConnToken &client) const;

    //! 设置/获取客户端连接的上下文数据
    using ContextDeleter = std::function<void(void*)>;
    void  setContext(const ConnToken &client, void *context, ContextDeleter &&deleter = nullptr);
    void* getContext(const ConnToken &client) const;

    void setContextLogEnable(bool enable);

    class Impl;

  private:
    Impl *impl_;
};

}
}
}

#endif //TBOX_HTTP_SSE_SERVER_H_20260616
