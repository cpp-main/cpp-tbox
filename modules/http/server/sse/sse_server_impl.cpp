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
#include "sse_server.h"
#include "sse_server_impl.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/defines.h>
#include <tbox/base/wrapped_recorder.h>

#include <tbox/network/tcp_connection.h>
#include <tbox/util/string.h>

#undef  MODULE_ID
#define MODULE_ID "tbox.http.sse"

namespace tbox {
namespace http {
namespace sse {

using namespace std::placeholders;

//! === 生命周期 ===

SseServer::Impl::Impl(SseServer *wp_parent, event::Loop *wp_loop)
  : wp_parent_(wp_parent)
  , wp_loop_(wp_loop)
  , sp_heartbeat_timer_(wp_loop->newTimerEvent())
{ }

SseServer::Impl::~Impl()
{
    TBOX_ASSERT(cb_level_ == 0);
    cleanup();
    CHECK_DELETE_RESET_OBJ(sp_heartbeat_timer_);
}

bool SseServer::Impl::initialize(http::server::Server *http_server, const std::string &url_path)
{
    if (state_ != SseServer::State::kNone)
        return false;

    //! 记录 URL 路径
    url_path_ = url_path;

    //! 记录 HTTP 服务器指针（不立即注册中间件，等 start() 时注册）
    wp_http_server_ = http_server;

    state_ = SseServer::State::kInited;
    return true;
}

bool SseServer::Impl::start()
{
    if (state_ != SseServer::State::kInited)
        return false;

    //! 注册自身到 HTTP 服务器（SseServer::Impl 即为 Middleware）
    mw_token_ = wp_http_server_->use(this);

    //! 如果心跳间隔已设置，启用心跳定时器
    if (heartbeat_interval_.count() > 0) {
        sp_heartbeat_timer_->initialize(heartbeat_interval_, event::Event::Mode::kPersist);
        sp_heartbeat_timer_->setCallback(std::bind(&SseServer::Impl::onHeartbeatTimer, this));
        sp_heartbeat_timer_->enable();
    }

    state_ = SseServer::State::kRunning;
    return true;
}

void SseServer::Impl::stop()
{
    if (state_ != SseServer::State::kRunning)
        return;

    //! 从 HTTP 服务器反注册中间件
    wp_http_server_->unuse(mw_token_);
    mw_token_.reset();

    //! 停止心跳定时器
    sp_heartbeat_timer_->disable();

    //! 清除 SseConnection 内部回调，防止断开时回调到 Impl
    sse_conns_.foreach([](SseConnection *conn) {
        conn->setCloseCallback(nullptr);
        conn->setSendCompleteCallback(nullptr);
    });

    //! 删除所有 SseConnection（析构时会断开并延后删除 TcpConnection）
    sse_conns_.foreach([](SseConnection *conn) { delete conn; });
    sse_conns_.clear();

    state_ = SseServer::State::kInited;
}

void SseServer::Impl::cleanup()
{
    if (state_ == SseServer::State::kNone)
        return;

    if (state_ == SseServer::State::kRunning)
        stop();

    wp_http_server_ = nullptr;

    connected_cb_ = nullptr;
    disconnected_cb_ = nullptr;
    heartbeat_interval_ = std::chrono::milliseconds(0);

    state_ = SseServer::State::kNone;
}

//! === Middleware 接口实现 ===
void SseServer::Impl::handle(http::server::ContextSptr sp_ctx, const http::server::NextFunc &next)
{
    auto &req = sp_ctx->req();

    if (!IsSseRequest(req)) {
        //! 非 SSE 请求，传递给下一个中间件
        next();
    }

    //! URL 路径匹配规则：
    //! - url_path_ 以 '/' 结尾：前缀匹配
    //! - url_path_ 不以 '/' 结尾：全量匹配
    //! - url_path_ 为空字符串：匹配所有 SSE 请求
    if (!url_path_.empty()) {
        bool matched = false;
        if (url_path_.back() == '/') {
            //! 前缀匹配
            matched = util::string::IsStartWith(req.url.path, url_path_);
        } else {
            //! 全量匹配
            matched = (req.url.path == url_path_);
        }
        if (!matched) {
            //! 不是本服务关心的 URL，传递给下一个中间件
            next();
            return;
        }
    }

    LogDbg("sse request: %s", req.url.path.c_str());

    auto &res = sp_ctx->res();

    //! 设置 200 OK 响应（SSE 不是协议升级，使用 200）
    res.status_code = http::StatusCode::k200_OK;
    res.http_ver = http::HttpVer::k1_1;

    //! SSE 必需的响应头
    res.headers["Content-Type"] = "text/event-stream";
    res.headers["Cache-Control"] = "no-cache";
    res.headers["Connection"] = "keep-alive";

    //! 从请求中提取 Last-Event-ID（浏览器重连时携带）
    std::string last_event_id;
    auto id_iter = http::FindHeader(req.headers, "last-event-id");
    if (id_iter != req.headers.end())
        last_event_id = id_iter->second;

    //! 注册升级回调：HTTP 服务器发送 200 响应后，将 TcpConnection 交给 SseServer
    //! 与 WebSocket 使用同一套 upgrade_cb 机制
    res.upgrade_cb = std::bind(&SseServer::Impl::onSseUpgrade, this, _1, req.url.path, last_event_id);

    //! SSE 请求已处理，不再调用 next()
}

//! === 升级与连接管理 ===
void SseServer::Impl::onSseUpgrade(network::TcpConnection *tcp_conn,
                                   const std::string &url_path,
                                   const std::string &last_event_id)
{
    RECORD_SCOPE();
    LogDbg("sse upgrade: new connection from %s", tcp_conn->peerAddr().toString().c_str());

    //! 创建 SseConnection，并存入 Cabinet
    //! 传入 URL 路径和 Last-Event-ID，供用户后续查询
    SseConnection *sse_conn = new SseConnection(wp_loop_, tcp_conn, url_path, last_event_id);
    ConnToken sse_token = sse_conns_.alloc(sse_conn);

    //! 设置 SseConnection 的回调（bind 捕获 ConnToken，不传递 SseConnection*）
    sse_conn->setCloseCallback(std::bind(&SseServer::Impl::onSseDisconnected, this, sse_token));
    sse_conn->setContextLogEnable(context_log_enable_);

    //! 通知用户（传递 ConnToken）
    if (connected_cb_) {
        ++cb_level_;
        connected_cb_(sse_token);
        --cb_level_;
    }
}

void SseServer::Impl::onSseDisconnected(const ConnToken &client)
{
    RECORD_SCOPE();
    LogDbg("sse disconnected");

    //! 先通知用户（此时 ConnToken 在 Cabinet 中仍有效）
    //! 用户可通过 ConnToken 调用 SseServer 方法获取连接信息
    if (disconnected_cb_) {
        ++cb_level_;
        disconnected_cb_(client);
        --cb_level_;
    }

    //! 从 Cabinet 中移除并获取指针
    SseConnection *sse_conn = sse_conns_.free(client);

    //! 延后删除 SseConnection（确保回调中还能访问对象）
    wp_loop_->runNext([sse_conn] { CHECK_DELETE_OBJ(sse_conn); },
        "SseServer::onSseDisconnected, delete sse_conn");
}

//! === 心跳定时器 ===
void SseServer::Impl::onHeartbeatTimer()
{
    //! 向所有连接发送心跳注释行
    sse_conns_.foreach([](SseConnection *conn) {
        conn->sendHeartbeat("keep-alive");
    });
}

void SseServer::Impl::setHeartbeatInterval(std::chrono::milliseconds interval)
{
    heartbeat_interval_ = interval;

    //! 如果已在运行中，动态调整心跳定时器
    if (state_ == SseServer::State::kRunning) {
        sp_heartbeat_timer_->disable();

        if (interval.count() > 0) {
            sp_heartbeat_timer_->initialize(interval, event::Event::Mode::kPersist);
            sp_heartbeat_timer_->setCallback(std::bind(&SseServer::Impl::onHeartbeatTimer, this));
            sp_heartbeat_timer_->enable();
        }
    }
}

//! === 通过 ConnToken 操作连接 ===
bool SseServer::Impl::send(const ConnToken &client, const std::string &data)
{
    auto sse_conn = sse_conns_.at(client);
    if (sse_conn != nullptr)
        return sse_conn->send(data);
    return false;
}

bool SseServer::Impl::send(const ConnToken &client, const SseEvent &event)
{
    auto sse_conn = sse_conns_.at(client);
    if (sse_conn != nullptr)
        return sse_conn->send(event);
    return false;
}

bool SseServer::Impl::sendToAll(const std::string &data)
{
    bool all_ok = true;
    sse_conns_.foreach([&](SseConnection *conn) {
        if (!conn->send(data))
            all_ok = false;
    });
    return all_ok;
}

bool SseServer::Impl::sendToAll(const SseEvent &event)
{
    bool all_ok = true;
    sse_conns_.foreach([&](SseConnection *conn) {
        if (!conn->send(event))
            all_ok = false;
    });
    return all_ok;
}

bool SseServer::Impl::close(const ConnToken &client)
{
    auto sse_conn = sse_conns_.at(client);
    if (sse_conn != nullptr)
        return sse_conn->close();
    return false;
}

bool SseServer::Impl::sendHeartbeat(const ConnToken &client, const std::string &comment)
{
    auto sse_conn = sse_conns_.at(client);
    if (sse_conn != nullptr)
        return sse_conn->sendHeartbeat(comment);
    return false;
}

bool SseServer::Impl::isClientValid(const ConnToken &client) const
{
    return sse_conns_.at(client) != nullptr;
}

network::SockAddr SseServer::Impl::peerAddr(const ConnToken &client) const
{
    auto sse_conn = sse_conns_.at(client);
    if (sse_conn != nullptr)
        return sse_conn->peerAddr();
    return network::SockAddr();
}

std::string SseServer::Impl::getLastEventId(const ConnToken &client) const
{
    auto sse_conn = sse_conns_.at(client);
    if (sse_conn != nullptr)
        return sse_conn->getLastEventId();
    return "";
}

std::string SseServer::Impl::getUrl(const ConnToken &client) const
{
    auto sse_conn = sse_conns_.at(client);
    if (sse_conn != nullptr)
        return sse_conn->getUrl();
    return "";
}

void SseServer::Impl::setContext(const ConnToken &client, void *context, ContextDeleter &&deleter)
{
    auto sse_conn = sse_conns_.at(client);
    if (sse_conn != nullptr)
        sse_conn->setContext(context, std::move(deleter));
}

void* SseServer::Impl::getContext(const ConnToken &client) const
{
    auto sse_conn = sse_conns_.at(client);
    if (sse_conn != nullptr)
        return sse_conn->getContext();
    return nullptr;
}

void SseServer::Impl::setContextLogEnable(bool enable)
{
    context_log_enable_ = enable;
    sse_conns_.foreach([&](SseConnection *conn) {
        conn->setContextLogEnable(enable);
    });
}

bool SseServer::Impl::IsSseRequest(const http::Request &req)
{
    //! SSE 请求检测条件：
    //! 1) 必须是 GET 方法
    //! 2) Accept 头必须包含 "text/event-stream"
    //! 3) 不检查 Upgrade 头（SSE 不是协议升级）

    if (req.method != http::Method::kGet)
        return false;

    auto accept_iter = http::FindHeader(req.headers, "accept");
    if (accept_iter == req.headers.end())
        return false;

    //! 检查 Accept 头是否包含 text/event-stream
    //! 注意：Accept 头可能包含多个值，如 "text/event-stream, text/html"
    if (accept_iter->second.find("text/event-stream") == std::string::npos)
        return false;

    return true;
}

//! === SseServer 外部接口 ===
SseServer::SseServer(event::Loop *wp_loop)
  : impl_(new Impl(this, wp_loop))
{
    TBOX_ASSERT(wp_loop != nullptr);
}

SseServer::~SseServer()
{
    CHECK_DELETE_RESET_OBJ(impl_);
}

bool SseServer::initialize(http::server::Server *http_server, const std::string &url_path)
{
    TBOX_ASSERT(http_server != nullptr);
    return impl_->initialize(http_server, url_path);
}

bool SseServer::start()
{
    return impl_->start();
}

void SseServer::stop()
{
    impl_->stop();
}

void SseServer::cleanup()
{
    impl_->cleanup();
}

SseServer::State SseServer::state() const
{
    return impl_->state();
}

void SseServer::setConnectedCallback(const ConnectedCallback &cb)
{
    impl_->setConnectedCallback(cb);
}

void SseServer::setDisconnectedCallback(const DisconnectedCallback &cb)
{
    impl_->setDisconnectedCallback(cb);
}

bool SseServer::send(const ConnToken &client, const std::string &data)
{
    return impl_->send(client, data);
}

bool SseServer::send(const ConnToken &client, const SseEvent &event)
{
    return impl_->send(client, event);
}

bool SseServer::sendToAll(const std::string &data)
{
    return impl_->sendToAll(data);
}

bool SseServer::sendToAll(const SseEvent &event)
{
    return impl_->sendToAll(event);
}

bool SseServer::close(const ConnToken &client)
{
    return impl_->close(client);
}

bool SseServer::sendHeartbeat(const ConnToken &client, const std::string &comment)
{
    return impl_->sendHeartbeat(client, comment);
}

void SseServer::setHeartbeatInterval(std::chrono::milliseconds interval)
{
    impl_->setHeartbeatInterval(interval);
}

bool SseServer::isClientValid(const ConnToken &client) const
{
    return impl_->isClientValid(client);
}

network::SockAddr SseServer::peerAddr(const ConnToken &client) const
{
    return impl_->peerAddr(client);
}

std::string SseServer::getLastEventId(const ConnToken &client) const
{
    return impl_->getLastEventId(client);
}

std::string SseServer::getUrl(const ConnToken &client) const
{
    return impl_->getUrl(client);
}

void SseServer::setContext(const ConnToken &client, void *context, ContextDeleter &&deleter)
{
    impl_->setContext(client, context, std::move(deleter));
}

void* SseServer::getContext(const ConnToken &client) const
{
    return impl_->getContext(client);
}

void SseServer::setContextLogEnable(bool enable)
{
    impl_->setContextLogEnable(enable);
}

}
}
}
