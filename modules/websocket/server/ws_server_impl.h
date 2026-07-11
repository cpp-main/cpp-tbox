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
#ifndef TBOX_WS_SERVER_IMPLH_20260612
#define TBOX_WS_SERVER_IMPLH_20260612

#include <tbox/event/loop.h>
#include <tbox/base/cabinet.hpp>
#include <tbox/base/defines.h>
#include <tbox/network/tcp_connection.h>
#include <tbox/network/sockaddr.h>

#include <tbox/http/server/server.h>
#include <tbox/http/server/middleware.h>
#include <tbox/http/server/context.h>
#include <tbox/http/request.h>

#include "ws_server.h"
#include "ws_connection.h"
#include "../ws_compressor.h"

namespace tbox {
namespace websocket {
namespace server {

//! WsServer::Impl 同时充当 HTTP 中间件
//! 检测 WebSocket 升级请求，设置 101 响应，注册 upgrade_cb
//! 通过 Cabinet 管理 WsConnection 生命期，所有操作基于 ConnToken
class WsServer::Impl : public http::server::Middleware {
  public:
    Impl(WsServer *wp_parent, event::Loop *wp_loop);
    virtual ~Impl();

  public:
    bool initialize(http::server::Server *http_server, const std::string &url_path = "");
    bool start();
    void stop();
    void cleanup();

    WsServer::State state() const { return state_; }

  public:
    void setConnectedCallback(const WsServer::ConnectedCallback &cb)       { connected_cb_ = cb; }
    void setDisconnectedCallback(const WsServer::DisconnectedCallback &cb) { disconnected_cb_ = cb; }
    void setTextMessageCallback(const WsServer::TextMessageCallback &cb)   { text_message_cb_ = cb; }
    void setBinaryMessageCallback(const WsServer::BinaryMessageCallback &cb) { binary_message_cb_ = cb; }
    void setErrorCallback(const WsServer::ErrorCallback &cb)               { error_cb_ = cb; }

    //! 压缩配置
    void setCompressionEnable(bool enable);

    //! 分片大小配置
    void setFragmentSize(size_t size) { fragment_size_ = size; }

    //! Ping/Pong 心跳配置
    void setPingInterval(int seconds) { ping_interval_ = seconds; }
    void setPingTimeout(int seconds) { ping_timeout_ = seconds; }

  public:
    //! 通过 ConnToken 操作连接（转发到 WsConnection）
    bool send(const ConnToken &client, const std::string &text);
    bool send(const ConnToken &client, const char *str);
    bool send(const ConnToken &client, const void *data, size_t len);
    bool send(const ConnToken &client, const std::vector<uint8_t> &data);
    bool close(const ConnToken &client, uint16_t code, const std::string &reason);
    bool ping(const ConnToken &client, const std::string &data);
    bool pong(const ConnToken &client, const std::string &data);
    bool isClientValid(const ConnToken &client) const;
    network::SockAddr peerAddr(const ConnToken &client) const;
    std::string getUrl(const ConnToken &client) const;

    //! 上下文数据操作（委托到 WsConnection → TcpConnection）
    using ContextDeleter = network::TcpConnection::ContextDeleter;
    void  setContext(const ConnToken &client, void *context, ContextDeleter &&deleter = nullptr);
    void* getContext(const ConnToken &client) const;

  public:
    //! Middleware 接口：处理 HTTP 请求，检测 WebSocket 升级
    virtual void handle(http::server::ContextSptr sp_ctx, const http::server::NextFunc &next) override;

    //! 静态辅助方法（供 WsServer 外部接口转发）
    static bool IsWsUpgradeRequest(const http::Request &req);
    static std::string ComputeWsAcceptKey(const std::string &sec_ws_key);

  private:
    //! 当 HTTP 服务器发送 101 响应后回调此函数
    void onWsUpgrade(network::TcpConnection *tcp_conn, const std::string &url_path,
                     const WsCompressionConfig &compress_config);

    //! 当 WsConnection 断开时回调（参数为 ConnToken）
    void onWsDisconnected(const ConnToken &client);

    //! 当 WsConnection 收到完整文本消息时回调
    void onWsTextMessage(const ConnToken &client, std::string &&data);

    //! 当 WsConnection 收到完整二进制消息时回调
    void onWsBinaryMessage(const ConnToken &client, std::vector<uint8_t> &&data);

    //! 当 WsConnection 出错时回调
    void onWsError(const ConnToken &client);

  private:
    WsServer *wp_parent_;
    event::Loop *wp_loop_;

    http::server::Server *wp_http_server_ = nullptr;
    //! URL 路径匹配规则：
    //! - url_path_ 以 '/' 结尾：前缀匹配，如 "/api/" 匹配 "/api/aa"
    //! - url_path_ 不以 '/' 结尾：全量匹配，如 "/api" 仅匹配 "/api"
    //! - url_path_ 为空字符串：匹配所有 WebSocket 升级请求
    std::string url_path_;

    //! 中间件 token（由 HTTP Server 的 use() 返回，用于 unuse() 反注册）
    http::server::MiddlewareToken mw_token_;

    //! 压缩配置
    WsCompressionConfig compression_config_;

    //! 分片发送的最大帧 payload 大小（可配置，默认 kDefaultFragmentSize）
    size_t fragment_size_ = WsServer::kDefaultFragmentSize;

    //! Ping/Pong 心跳参数
    int ping_interval_ = 0;
    int ping_timeout_ = 0;

    //! WsConnection 容器（生命期管理）
    cabinet::Cabinet<WsConnection> ws_conns_;

    WsServer::State state_ = WsServer::State::kNone;

    WsServer::ConnectedCallback    connected_cb_;
    WsServer::DisconnectedCallback disconnected_cb_;
    WsServer::TextMessageCallback  text_message_cb_;
    WsServer::BinaryMessageCallback binary_message_cb_;
    WsServer::ErrorCallback        error_cb_;

    int cb_level_ = 0;
};

}
}
}
#endif //TBOX_WS_SERVER_IMPLH_20260612
