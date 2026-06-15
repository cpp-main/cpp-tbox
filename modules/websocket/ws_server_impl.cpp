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
#include "ws_server.h"
#include "ws_server_impl.h"

#include <tbox/base/log.h>
#include <tbox/base/defines.h>
#include <tbox/base/wrapped_recorder.h>

#include <tbox/network/tcp_connection.h>
#include <tbox/crypto/sha1.h>
#include <tbox/util/base64.h>

#undef  MODULE_ID
#define MODULE_ID "tbox.ws"

namespace tbox {
namespace websocket {

using namespace std::placeholders;

WsServer::Impl::Impl(WsServer *wp_parent, event::Loop *wp_loop) :
    wp_parent_(wp_parent),
    wp_loop_(wp_loop)
{ }

WsServer::Impl::~Impl()
{
    cleanup();
}

bool WsServer::Impl::initialize(http::server::Server *http_server, const std::string &url_path)
{
    if (state_ != WsServer::State::kNone)
        return false;

    //! 记录 URL 路径
    url_path_ = url_path;

    //! 记录 HTTP 服务器指针（不立即注册中间件，等 start() 时注册）
    wp_http_server_ = http_server;

    state_ = WsServer::State::kInited;
    return true;
}

bool WsServer::Impl::start()
{
    if (state_ != WsServer::State::kInited)
        return false;

    //! 注册自身到 HTTP 服务器（WsServer::Impl 即为 Middleware）
    mw_token_ = wp_http_server_->use(this);

    state_ = WsServer::State::kRunning;
    return true;
}

void WsServer::Impl::stop()
{
    if (state_ != WsServer::State::kRunning)
        return;

    //! 从 HTTP 服务器反注册中间件
    wp_http_server_->unuse(mw_token_);
    mw_token_.reset();

    //! 清除 WsConnection 内部回调，防止断开时回调到 Impl
    ws_conns_.foreach([](WsConnection *conn) {
        conn->setCloseCallback(nullptr);
        conn->setMessageCallback(nullptr);
        conn->setErrorCallback(nullptr);
    });

    //! 删除所有 WsConnection（析构时会断开并延后删除 TcpConnection）
    ws_conns_.foreach([](WsConnection *conn) {
        delete conn;
    });
    ws_conns_.clear();

    state_ = WsServer::State::kInited;
}

void WsServer::Impl::cleanup()
{
    if (state_ == WsServer::State::kNone)
        return;

    if (state_ == WsServer::State::kRunning)
        stop();

    wp_http_server_ = nullptr;

    state_ = WsServer::State::kNone;
}

//! === Middleware 接口实现 ===

void WsServer::Impl::handle(http::server::ContextSptr sp_ctx, const http::server::NextFunc &next)
{
    auto &req = sp_ctx->req();

    if (IsWsUpgradeRequest(req)) {
        //! URL 路径匹配（精确匹配）
        if (!url_path_.empty() && req.url.path != url_path_) {
            //! 不是本服务关心的 URL，传递给下一个中间件
            next();
            return;
        }

        LogDbg("ws upgrade request: %s", req.url.path.c_str());

        auto &res = sp_ctx->res();

        //! 设置 101 响应
        res.status_code = http::StatusCode::k101_SwitchingProtocols;
        res.http_ver = http::HttpVer::k1_1;

        //! 从请求头中获取 Upgrade 和 Connection 信息
        auto upgrade_iter = req.headers.find("Upgrade");
        if (upgrade_iter != req.headers.end())
            res.headers["Upgrade"] = upgrade_iter->second;
        else
            res.headers["Upgrade"] = "websocket";

        auto connection_iter = req.headers.find("Connection");
        if (connection_iter != req.headers.end())
            res.headers["Connection"] = connection_iter->second;
        else
            res.headers["Connection"] = "Upgrade";

        //! 计算 Sec-WebSocket-Accept
        auto key_iter = req.headers.find("Sec-WebSocket-Key");
        if (key_iter != req.headers.end())
            res.headers["Sec-WebSocket-Accept"] = ComputeWsAcceptKey(key_iter->second);

        //! 注册升级回调：HTTP 服务器发送 101 响应后，将 TcpConnection 交给 WsServer
        res.upgrade_cb = std::bind(&WsServer::Impl::onWsUpgrade, this, _1);

        //! 升级请求已处理，不再调用 next()
        return;
    }

    //! 非 WebSocket 升级请求，传递给下一个中间件
    next();
}

//! === 升级与连接管理 ===

void WsServer::Impl::onWsUpgrade(network::TcpConnection *tcp_conn)
{
    RECORD_SCOPE();
    LogDbg("ws upgrade: new connection from %s", tcp_conn->peerAddr().toString().c_str());

    //! 创建 WsConnection，并存入 Cabinet（直接 alloc 并存入指针）
    WsConnection *ws_conn = new WsConnection(wp_loop_, tcp_conn);
    ConnToken ws_token = ws_conns_.alloc(ws_conn);

    //! 设置 WsConnection 的回调（bind 捕获 ConnToken，不传递 WsConnection*）
    ws_conn->setCloseCallback(std::bind(&WsServer::Impl::onWsDisconnected, this, ws_token));
    ws_conn->setMessageCallback(std::bind(&WsServer::Impl::onWsMessage, this, ws_token, _1));
    ws_conn->setErrorCallback(std::bind(&WsServer::Impl::onWsError, this, ws_token));

    //! 通知用户（传递 ConnToken）
    if (connected_cb_)
        connected_cb_(ws_token);
}

void WsServer::Impl::onWsDisconnected(const ConnToken &client)
{
    RECORD_SCOPE();
    LogDbg("ws disconnected");

    //! 先通知用户（此时 ConnToken 在 Cabinet 中仍有效）
    //! 用户可通过 ConnToken 调用 WsServer 方法获取连接信息
    if (disconnected_cb_)
        disconnected_cb_(client);

    //! 从 Cabinet 中移除并获取指针
    WsConnection *ws_conn = ws_conns_.free(client);

    //! 延后删除 WsConnection（确保回调中还能访问对象）
    wp_loop_->runNext([ws_conn] { CHECK_DELETE_OBJ(ws_conn); },
        "WsServer::onWsDisconnected, delete ws_conn");
}

void WsServer::Impl::onWsMessage(const ConnToken &client, const WsFrame &frame)
{
    if (message_cb_)
        message_cb_(client, frame);
}

void WsServer::Impl::onWsError(const ConnToken &client)
{
    if (error_cb_)
        error_cb_(client);

    //! 出错后关闭连接
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        ws_conn->close();
}

//! === 通过 ConnToken 操作连接 ===

bool WsServer::Impl::send(const ConnToken &client, const std::string &text)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->send(text);
    return false;
}

bool WsServer::Impl::send(const ConnToken &client, const void *data, size_t len)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->send(data, len);
    return false;
}

bool WsServer::Impl::sendBinary(const ConnToken &client, const std::vector<uint8_t> &data)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->sendBinary(data);
    return false;
}

bool WsServer::Impl::close(const ConnToken &client, uint16_t code, const std::string &reason)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->close(code, reason);
    return false;
}

bool WsServer::Impl::ping(const ConnToken &client, const std::string &data)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->ping(data);
    return false;
}

bool WsServer::Impl::pong(const ConnToken &client, const std::string &data)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->pong(data);
    return false;
}

bool WsServer::Impl::isClientValid(const ConnToken &client) const
{
    return ws_conns_.at(client) != nullptr;
}

network::SockAddr WsServer::Impl::peerAddr(const ConnToken &client) const
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->peerAddr();
    return network::SockAddr();
}

//! === 静态辅助方法 ===

bool WsServer::Impl::IsWsUpgradeRequest(const http::Request &req)
{
    //! RFC 6455 Section 4.1:
    //! 1) 必须是 GET 方法
    //! 2) 必须包含 Upgrade: websocket 头部
    //! 3) 必须包含 Connection: Upgrade 头部
    //! 4) 必须包含 Sec-WebSocket-Key 头部
    //! 5) 必须包含 Sec-WebSocket-Version: 13 头部

    if (req.method != http::Method::kGet)
        return false;

    auto upgrade_iter = req.headers.find("Upgrade");
    if (upgrade_iter == req.headers.end()
        || upgrade_iter->second.find("websocket") == std::string::npos)
        return false;

    auto connection_iter = req.headers.find("Connection");
    if (connection_iter == req.headers.end()
        || connection_iter->second.find("Upgrade") == std::string::npos)
        return false;

    if (req.headers.find("Sec-WebSocket-Key") == req.headers.end())
        return false;

    //! 检查版本号
    auto version_iter = req.headers.find("Sec-WebSocket-Version");
    if (version_iter == req.headers.end()
        || version_iter->second != "13")
        return false;

    return true;
}

std::string WsServer::Impl::ComputeWsAcceptKey(const std::string &sec_ws_key)
{
    //! RFC 6455 Section 4.2.2:
    //! Sec-WebSocket-Accept = Base64(SHA1(Sec-WebSocket-Key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"))
    static const std::string ws_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    std::string combined = sec_ws_key + ws_guid;
    uint8_t digest[20];
    crypto::SHA1::Calc(combined.data(), combined.size(), digest);

    return util::base64::Encode(digest, 20);
}

//! === WsServer 外部接口 ===

WsServer::WsServer(event::Loop *wp_loop)
  : impl_(new Impl(this, wp_loop))
{ }

WsServer::~WsServer()
{
    CHECK_DELETE_RESET_OBJ(impl_);
}

bool WsServer::initialize(http::server::Server *http_server, const std::string &url_path)
{
    return impl_->initialize(http_server, url_path);
}

bool WsServer::start()
{
    return impl_->start();
}

void WsServer::stop()
{
    impl_->stop();
}

void WsServer::cleanup()
{
    impl_->cleanup();
}

WsServer::State WsServer::state() const
{
    return impl_->state();
}

void WsServer::setConnectedCallback(const ConnectedCallback &cb)
{
    impl_->setConnectedCallback(cb);
}

void WsServer::setDisconnectedCallback(const DisconnectedCallback &cb)
{
    impl_->setDisconnectedCallback(cb);
}

void WsServer::setMessageCallback(const MessageCallback &cb)
{
    impl_->setMessageCallback(cb);
}

void WsServer::setErrorCallback(const ErrorCallback &cb)
{
    impl_->setErrorCallback(cb);
}

bool WsServer::send(const ConnToken &client, const std::string &text)
{
    return impl_->send(client, text);
}

bool WsServer::send(const ConnToken &client, const void *data, size_t len)
{
    return impl_->send(client, data, len);
}

bool WsServer::sendBinary(const ConnToken &client, const std::vector<uint8_t> &data)
{
    return impl_->sendBinary(client, data);
}

bool WsServer::close(const ConnToken &client, uint16_t code, const std::string &reason)
{
    return impl_->close(client, code, reason);
}

bool WsServer::ping(const ConnToken &client, const std::string &data)
{
    return impl_->ping(client, data);
}

bool WsServer::pong(const ConnToken &client, const std::string &data)
{
    return impl_->pong(client, data);
}

bool WsServer::isClientValid(const ConnToken &client) const
{
    return impl_->isClientValid(client);
}

network::SockAddr WsServer::peerAddr(const ConnToken &client) const
{
    return impl_->peerAddr(client);
}

bool WsServer::IsWsUpgradeRequest(const http::Request &req)
{
    return Impl::IsWsUpgradeRequest(req);
}

std::string WsServer::ComputeWsAcceptKey(const std::string &sec_ws_key)
{
    return Impl::ComputeWsAcceptKey(sec_ws_key);
}

}
}
