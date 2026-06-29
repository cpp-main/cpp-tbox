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
#ifndef TBOX_HTTP_SSE_SERVER_IMPL_H_20260616
#define TBOX_HTTP_SSE_SERVER_IMPL_H_20260616

#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/base/cabinet.hpp>
#include <tbox/base/defines.h>
#include <tbox/network/tcp_connection.h>
#include <tbox/network/sockaddr.h>

#include "../server.h"
#include "../middleware.h"
#include "../context.h"

#include "sse_server.h"
#include "sse_connection.h"

namespace tbox {
namespace http {
namespace sse {

//! SseServer::Impl 同时充当 HTTP 中间件
//! 检测 SSE 请求（Accept: text/event-stream），设置 200 响应头，注册 upgrade_cb
//! 通过 Cabinet 管理 SseConnection 生命期，所有操作基于 ConnToken
//! SSE 是单向推送协议，无 MessageCallback
class SseServer::Impl : public http::server::Middleware {
  public:
    Impl(SseServer *wp_parent, event::Loop *wp_loop);
    virtual ~Impl();

  public:
    bool initialize(http::server::Server *http_server, const std::string &url_path = "");
    bool start();
    void stop();
    void cleanup();

    SseServer::State state() const { return state_; }

  public:
    void setConnectedCallback(const SseServer::ConnectedCallback &cb) { connected_cb_ = cb; }
    void setDisconnectedCallback(const SseServer::DisconnectedCallback &cb) { disconnected_cb_ = cb; }

  public:
    //! 通过 ConnToken 操作连接
    bool send(const ConnToken &client, const std::string &data);
    bool send(const ConnToken &client, const SseEvent &event);
    bool sendToAll(const std::string &data);
    bool sendToAll(const SseEvent &event);
    bool close(const ConnToken &client);
    bool sendHeartbeat(const ConnToken &client, const std::string &comment);
    bool isClientValid(const ConnToken &client) const;
    network::SockAddr peerAddr(const ConnToken &client) const;
    std::string getLastEventId(const ConnToken &client) const;
    std::string getUrl(const ConnToken &client) const;
    void setHeartbeatInterval(std::chrono::milliseconds interval);

    //! 上下文数据操作（委托到 SseConnection → TcpConnection）
    using ContextDeleter = network::TcpConnection::ContextDeleter;
    void  setContext(const ConnToken &client, void *context, ContextDeleter &&deleter = nullptr);
    void* getContext(const ConnToken &client) const;

    void setContextLogEnable(bool enable);

    //! 静态辅助方法
    static bool IsSseRequest(const http::Request &req);

  public:
    //! Middleware 接口：处理 HTTP 请求，检测 SSE 请求
    virtual void handle(http::server::ContextSptr sp_ctx, const http::server::NextFunc &next) override;

  private:
    //! 当 HTTP 服务器发送 200 响应后回调此函数
    //! 将 TcpConnection 从 HTTP 服务器分离，交给 SseServer 管理
    void onSseUpgrade(network::TcpConnection *tcp_conn,
                      const std::string &url_path,
                      const std::string &last_event_id);

    //! 当 SseConnection 断开时回调（参数为 ConnToken）
    void onSseDisconnected(const ConnToken &client);

    //! 心跳定时器回调：向所有连接发送注释行
    void onHeartbeatTimer();

  private:
    SseServer *wp_parent_;
    event::Loop *wp_loop_;
    event::TimerEvent *sp_heartbeat_timer_ = nullptr;   //! 心跳定时器（可选，默认禁用）
    http::server::Server *wp_http_server_ = nullptr;

    //! URL 路径匹配规则：
    //! - url_path_ 以 '/' 结尾：前缀匹配，如 "/sse/" 匹配 "/sse/aa"
    //! - url_path_ 不以 '/' 结尾：全量匹配，如 "/sse" 仅匹配 "/sse"
    //! - url_path_ 为空字符串：匹配所有 SSE 请求
    std::string url_path_;

    //! 中间件 token（由 HTTP Server 的 use() 返回，用于 unuse() 反注册）
    http::server::MiddlewareToken mw_token_;

    //! SseConnection 容器（生命期管理）
    cabinet::Cabinet<SseConnection> sse_conns_;

    std::chrono::milliseconds heartbeat_interval_{0};

    SseServer::State state_ = SseServer::State::kNone;

    SseServer::ConnectedCallback    connected_cb_;
    SseServer::DisconnectedCallback disconnected_cb_;

    int cb_level_ = 0;
    bool context_log_enable_ = false;
};

}
}
}

#endif //TBOX_HTTP_SSE_SERVER_IMPL_H_20260616
