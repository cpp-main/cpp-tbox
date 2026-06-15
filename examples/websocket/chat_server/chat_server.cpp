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
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/signal_event.h>
#include <tbox/http/server/server.h>
#include <tbox/websocket/server/ws_server.h>

#include <set>
#include <string>

#include "html_text.h"

using namespace tbox;
using namespace tbox::event;
using namespace tbox::http;
using namespace tbox::http::server;
using namespace tbox::websocket;
using namespace tbox::websocket::server;

//! 群聊聊天室
//! 内含 WsServer，统一管理 WebSocket 连接与聊天逻辑
//! 第一条文本消息为用户名（登录），之后为聊天消息
class ChatRoom {
  public:
    ChatRoom(event::Loop *wp_loop, const std::string &name)
      : wp_loop_(wp_loop)
      , name_(name)
      , ws_srv_(wp_loop)
    { }

    bool initialize(http::server::Server *http_srv, const std::string &url_path)
    {
        if (!ws_srv_.initialize(http_srv, url_path))
            return false;

        ws_srv_.setConnectedCallback([this](const WsServer::ConnToken &token) {
            onConnected(token);
        });
        ws_srv_.setDisconnectedCallback([this](const WsServer::ConnToken &token) {
            onDisconnected(token);
        });
        ws_srv_.setMessageCallback([this](const WsServer::ConnToken &token, const WsFrame &frame) {
            onMessage(token, frame);
        });

        LogInfo("chat room '%s' mounted at %s", name_.c_str(), url_path.c_str());
        return true;
    }

    bool start() { return ws_srv_.start(); }
    void stop()  { ws_srv_.stop(); }
    void cleanup()
    {
        ws_srv_.cleanup();
        conns_.clear();
        conn_to_name_.clear();
    }

  private:
    //! 连接建立：暂不广播，等收到用户名后再广播上线
    void onConnected(const WsServer::ConnToken &token)
    {
        auto url_path = ws_srv_.getUrl(token);
        LogInfo("url_path:%s", url_path.c_str());

        conns_.insert(token);
    }

    //! 连接断开：若已登录则广播下线消息
    void onDisconnected(const WsServer::ConnToken &token)
    {
        auto it = conn_to_name_.find(token);
        if (it != conn_to_name_.end()) {
            std::string name = it->second;
            conn_to_name_.erase(token);
            conns_.erase(token);
            LogInfo("[%s] user '%s' offline", name_.c_str(), name.c_str());
            broadcast(name + " 下线");
        } else {
            conns_.erase(token);
        }
    }

    //! 收到消息：第一条为用户名（登录），后续为聊天消息
    void onMessage(const WsServer::ConnToken &token, const WsFrame &frame)
    {
        if (frame.opcode != WsFrame::OpCode::kText)
            return;

        auto it = conn_to_name_.find(token);
        if (it == conn_to_name_.end()) {
            //! 第一条消息作为用户名
            conn_to_name_[token] = frame.payload;
            LogInfo("[%s] user '%s' online", name_.c_str(), frame.payload.c_str());
            broadcast(frame.payload + " 上线");
        } else {
            broadcast(it->second + ": " + frame.payload);
        }
    }

    //! 仅向已登录的用户广播（有用户名的连接）
    void broadcast(const std::string &msg)
    {
        for (const auto &pair : conn_to_name_)
            ws_srv_.send(pair.first, msg);
    }

  private:
    event::Loop *wp_loop_;
    std::string name_;
    WsServer ws_srv_;
    std::set<WsServer::ConnToken> conns_;
    std::map<WsServer::ConnToken, std::string> conn_to_name_;
};

int main(int argc, char **argv)
{
    std::string bind_addr = "0.0.0.0:8080";

    if (argc == 2)
        bind_addr = argv[1];

    LogOutput_Enable();

    LogInfo("enter");

    auto sp_loop = Loop::New();
    auto sp_sig_event = sp_loop->newSignalEvent();

    SetScopeExitAction(
        [=] {
            delete sp_sig_event;
            delete sp_loop;
        }
    );

    sp_sig_event->initialize(SIGINT, Event::Mode::kPersist);
    sp_sig_event->enable();

    //! 创建 HTTP 服务器
    Server http_srv(sp_loop);
    if (!http_srv.initialize(network::SockAddr::FromString(bind_addr), 1)) {
        LogErr("init http server fail");
        return 0;
    }

    //! 创建两个聊天室，分别挂载到 /ws/chat-1 和 /ws/chat-2
    ChatRoom chat_room_1(sp_loop, "聊天室1");
    ChatRoom chat_room_2(sp_loop, "聊天室2");

    if (!chat_room_1.initialize(&http_srv, "/ws/chat-1")) {
        LogErr("init chat room 1 fail");
        return 0;
    }
    if (!chat_room_2.initialize(&http_srv, "/ws/chat-2")) {
        LogErr("init chat room 2 fail");
        return 0;
    }

    //! 添加 HTTP 请求处理（主页面）
    http_srv.use(
        [&](ContextSptr ctx, const NextFunc &next) {
            if (ctx->req().url.path == "/") {
                ctx->res().status_code = StatusCode::k200_OK;
                ctx->res().headers["Content-Type"] = "text/html; charset=utf-8";
                ctx->res().body = kChatHtml;
                return;
            }
            next();
        }
    );

    //! 启动服务
    http_srv.start();
    chat_room_1.start();
    chat_room_2.start();

    //! Ctrl+C 退出
    sp_sig_event->setCallback(
        [&] (int) {
            chat_room_1.stop();
            chat_room_2.stop();
            http_srv.stop();
            sp_loop->exitLoop();
        }
    );

    LogInfo("start, listen at %s", bind_addr.c_str());
    sp_loop->runLoop();
    LogInfo("stop");

    chat_room_1.cleanup();
    chat_room_2.cleanup();
    http_srv.cleanup();

    LogInfo("exit");
    return 0;
}
