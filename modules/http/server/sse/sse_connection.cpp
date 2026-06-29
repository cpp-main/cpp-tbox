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
#include "sse_connection.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/util/buffer.h>

namespace tbox {
namespace http {
namespace sse {

using namespace std::placeholders;

SseConnection::SseConnection(event::Loop *wp_loop,
                             network::TcpConnection *tcp_conn,
                             const std::string &url,
                             const std::string &last_event_id)
  : wp_loop_(wp_loop)
  , sp_tcp_conn_(tcp_conn)
  , url_(url)
  , last_event_id_(last_event_id)
{
    TBOX_ASSERT(wp_loop != nullptr);
    TBOX_ASSERT(tcp_conn != nullptr);

    //! 设置 TcpConnection 的回调
    //! SSE 是单向推送协议（服务端→客户端），不需要处理客户端发送的数据
    //! 但必须保持 receiveCallback 注册（阈值=0），否则底层 BufferedFd 会停止监听
    //! socket 读事件，导致：(1) 无法检测浏览器关闭连接；(2) TCP 写事件也可能受影响
    //! 与 WsConnection 一样，设置空函数体回调而非 nullptr，确保读事件持续监听
    sp_tcp_conn_->setReceiveCallback([](util::Buffer &buff) { buff.hasReadAll(); }, 0);
    sp_tcp_conn_->setDisconnectedCallback(std::bind(&SseConnection::onTcpDisconnected, this));
    sp_tcp_conn_->setSendCompleteCallback(std::bind(&SseConnection::onTcpSendCompleted, this));
}

SseConnection::~SseConnection()
{
    TBOX_ASSERT(cb_level_ == 0);

    if (sp_tcp_conn_ == nullptr)
        return;

    //! 先取消 TcpConnection 的回调，防止断开时回调到已销毁的 SseConnection
    //! 注意：receiveCallback 不能设为 nullptr，否则会停止 socket 读事件监听
    //! 设置空函数体回调即可
    sp_tcp_conn_->setDisconnectedCallback(nullptr);
    sp_tcp_conn_->setSendCompleteCallback(nullptr);

    sp_tcp_conn_->disconnect();
    auto tcp_conn = sp_tcp_conn_;
    sp_tcp_conn_ = nullptr;
    wp_loop_->runNext([tcp_conn] { CHECK_DELETE_OBJ(tcp_conn); },
        "SseConnection::~SseConnection, delete tcp_conn");
}

//! === 发送事件 ===

bool SseConnection::send(const std::string &data)
{
    if (sp_tcp_conn_ == nullptr)
        return false;

    //! 简单数据：只输出 data 字段
    //! 格式："data: xxx\n\n"
    std::string sse_text = "data: " + data + "\n\n";

    if (context_log_enable_)
      LogDbg("SEND: %s", sse_text.c_str());

    return sp_tcp_conn_->send(sse_text.data(), sse_text.size());
}

bool SseConnection::send(const SseEvent &event)
{
    if (sp_tcp_conn_ == nullptr)
        return false;

    //! 完整事件：调用 SseEvent::toString() 格式化后发送
    std::string sse_text = event.toString();

    if (context_log_enable_)
      LogDbg("SEND: %s", sse_text.c_str());

    return sp_tcp_conn_->send(sse_text.data(), sse_text.size());
}

bool SseConnection::sendHeartbeat(const std::string &comment)
{
    if (sp_tcp_conn_ == nullptr)
        return false;

    //! 心跳注释行：": <comment>\n\n"
    //! 浏览器 EventSource 会忽略以 ":" 开头的行，用于保持连接活跃
    std::string sse_text = ": " + comment + "\n\n";

    if (context_log_enable_)
      LogDbg("SEND: %s", sse_text.c_str());

    return sp_tcp_conn_->send(sse_text.data(), sse_text.size());
}

bool SseConnection::close()
{
    if (sp_tcp_conn_ == nullptr)
        return false;

    //! SSE 没有特殊的关闭协议，直接断开 TCP 连接
    sp_tcp_conn_->disconnect();
    return true;
}

//! === 客户端信息 ===

network::SockAddr SseConnection::peerAddr() const
{
    if (sp_tcp_conn_ != nullptr)
        return sp_tcp_conn_->peerAddr();
    return network::SockAddr();
}

bool SseConnection::isExpired() const
{
    return sp_tcp_conn_ == nullptr || sp_tcp_conn_->isExpired();
}

//! === 上下文数据 ===

void SseConnection::setContext(void *context, ContextDeleter &&deleter)
{
    if (sp_tcp_conn_ != nullptr)
        sp_tcp_conn_->setContext(context, std::move(deleter));
}

void* SseConnection::getContext() const
{
    if (sp_tcp_conn_ != nullptr)
        return sp_tcp_conn_->getContext();
    return nullptr;
}

//! === TCP 回调 ===

void SseConnection::onTcpDisconnected()
{
    LogInfo("sse disconnected");

    //! 通知 SseServer（通过 close_cb_ 绑定了 ConnToken）
    if (close_cb_) {
        ++cb_level_;
        close_cb_();
        --cb_level_;
    }

    //! 清理 TcpConnection：先断空指针，延后删除
    //! 必须在 close_cb_() 之后清理，否则回调中 getContext() 拿到空值
    auto tcp_conn = sp_tcp_conn_;
    sp_tcp_conn_ = nullptr;
    wp_loop_->runNext([tcp_conn] { CHECK_DELETE_OBJ(tcp_conn); },
        "SseConnection::onTcpDisconnected, delete tcp_conn");
}

void SseConnection::onTcpSendCompleted()
{
    if (send_complete_cb_) {
        ++cb_level_;
        send_complete_cb_();
        --cb_level_;
    }
}

}
}
}
