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
#ifndef TBOX_HTTP_SSE_CONNECTION_H_20260616
#define TBOX_HTTP_SSE_CONNECTION_H_20260616

#include <tbox/event/loop.h>
#include <tbox/network/tcp_connection.h>
#include <tbox/base/defines.h>

#include "sse_event.h"

namespace tbox {
namespace http {
namespace sse {

//! SSE 连接
//! 包装从 HTTP 升级后分离出来的 TcpConnection，提供 SSE 事件推送功能
//! 生命期由 SseServer 通过 Cabinet 管理，用户通过 ConnToken 访问
//! SSE 是单向推送协议（服务端→客户端），不需要解析客户端数据
class SseConnection {
  public:
    //! 内部回调：SseServer::Impl 绑定 ConnToken，不传递 SseConnection*
    using CloseCallback        = std::function<void()>;
    using SendCompleteCallback = std::function<void()>;

    ~SseConnection();

    NONCOPYABLE(SseConnection);
    IMMOVABLE(SseConnection);

  public:
    //! 设置回调（由 SseServer::Impl 调用，绑定 ConnToken）
    void setCloseCallback(const CloseCallback &cb) { close_cb_ = cb; }
    void setSendCompleteCallback(const SendCompleteCallback &cb) { send_complete_cb_ = cb; }
    void setContextLogEnable(bool enable) { context_log_enable_ = enable; }

  public:
    //! 发送简单数据（event 类型默认 "message"）
    bool send(const std::string &data);

    //! 发送完整 SSE 事件
    bool send(const SseEvent &event);

    //! 发送心跳注释行（保持连接活跃）
    //! 格式：": <comment>\n\n"
    bool sendHeartbeat(const std::string &comment = "keep-alive");

    //! 关闭连接（断开 TcpConnection）
    bool close();

    //! 获取客户端地址
    network::SockAddr peerAddr() const;

    //! 获取浏览器重连时携带的 Last-Event-ID
    std::string getLastEventId() const { return last_event_id_; }

    //! 获取客户端连接的 URL 路径
    std::string getUrl() const { return url_; }

    //! 连接是否已失效
    bool isExpired() const;

    //! 设置/获取上下文数据（委托给底层 TcpConnection）
    using ContextDeleter = network::TcpConnection::ContextDeleter;
    void  setContext(void *context, ContextDeleter &&deleter = nullptr);
    void* getContext() const;

  private:
    //! 仅由 SseServer 创建（生命期由 Cabinet 管理）
    SseConnection(event::Loop *wp_loop,
                  network::TcpConnection *tcp_conn,
                  const std::string &url,
                  const std::string &last_event_id);

    void onTcpDisconnected();
    void onTcpSendCompleted();

  private:
    event::Loop *wp_loop_;
    network::TcpConnection *sp_tcp_conn_;
    std::string url_;
    std::string last_event_id_;  //! 浏览器重连时的 Last-Event-ID

    CloseCallback        close_cb_;
    SendCompleteCallback send_complete_cb_;

    int cb_level_ = 0;
    bool context_log_enable_ = false;

    friend class SseServer;
};

}
}
}

#endif //TBOX_HTTP_SSE_CONNECTION_H_20260616
