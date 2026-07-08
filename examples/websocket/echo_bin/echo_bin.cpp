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

/**
 * WebSocket 二进制 Echo 示例
 *
 * 功能：
 * - 客户端发送二进制数据帧，服务器原样回传（echo）
 * - 服务器统计收发帧数与字节数
 * - 服务器每 5 秒向所有客户端推送二进制统计帧（4字节头"STAT" + JSON字符串）
 *
 * 演示要点：
 * - WsServer::send() 的 void* + len 版本：发送原始二进制
 * - WsServer::sendBinary() 的 vector<uint8_t> 版本：发送 vector 二进制
 * - WsFrame::OpCode::kBinary：区分文本帧与二进制帧
 * - event::TimerEvent：定时推送统计数据
 * - WsServer 的 start()/stop() 生命周期
 */

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/signal_event.h>
#include <tbox/event/timer_event.h>
#include <tbox/http/server/server.h>
#include <tbox/websocket/server/ws_server.h>

#include <set>
#include <string>
#include <chrono>
#include <vector>

#include "html_text.h"

using namespace tbox;
using namespace tbox::event;
using namespace tbox::http;
using namespace tbox::http::server;
using namespace tbox::websocket;
using namespace tbox::websocket::server;

//! 统计帧的头部标识：0x53 0x54 0x41 0x54 = "STAT"
static const uint8_t kStatHeader[4] = {0x53, 0x54, 0x41, 0x54};

//! 二进制 Echo 服务
//! 内含 WsServer + 统计信息 + 定时器推送
class EchoService {
  public:
    EchoService(Loop *wp_loop)
      : wp_loop_(wp_loop)
      , ws_srv_(wp_loop)
      , stat_timer_(wp_loop->newTimerEvent())
    { }

    ~EchoService()
    {
        CHECK_DELETE_RESET_OBJ(stat_timer_);
    }

    bool initialize(Server *http_srv, const std::string &url_path)
    {
        //! 初始化 WsServer，指定 URL 路径
        if (!ws_srv_.initialize(http_srv, url_path))
            return false;

        //! 设置回调
        ws_srv_.setConnectedCallback([this](const WsServer::ConnToken &token) {
            onConnected(token);
        });
        ws_srv_.setDisconnectedCallback([this](const WsServer::ConnToken &token) {
            onDisconnected(token);
        });
        ws_srv_.setMessageCallback([this](const WsServer::ConnToken &token, const WsFrame &frame) {
            onMessage(token, frame);
        });
        ws_srv_.setCompressionEnable(true);

        //! 初始化定时器：每 5 秒推送统计帧
        stat_timer_->initialize(std::chrono::milliseconds(5000), Event::Mode::kPersist);
        stat_timer_->setCallback([this] { onStatTimer(); });

        LogInfo("echo service mounted at %s", url_path.c_str());
        return true;
    }

    bool start()
    {
        if (!ws_srv_.start())
            return false;

        stat_timer_->enable();
        return true;
    }

    void stop()
    {
        stat_timer_->disable();
        ws_srv_.stop();
    }

    void cleanup()
    {
        ws_srv_.cleanup();
        conns_.clear();
    }

  private:
    //! 新连接：记录 token
    void onConnected(const WsServer::ConnToken &token)
    {
        conns_.insert(token);
        LogDbg("client connected, total: %d", conns_.size());
    }

    //! 断开连接：移除 token
    void onDisconnected(const WsServer::ConnToken &token)
    {
        conns_.erase(token);
        LogDbg("client disconnected, total: %d", conns_.size());
    }

    //! 收到消息：区分文本帧与二进制帧
    void onMessage(const WsServer::ConnToken &token, const WsFrame &frame)
    {
        if (frame.opcode == WsFrame::OpCode::kBinary) {
            //! 二进制帧：echo 回传原数据
            //! 演示 WsServer::send() 的 void* + len 版本
            ws_srv_.send(token, frame.payload.data(), frame.payload.size());

            //! 更新统计
            recv_frames_++;
            recv_bytes_ += frame.payload.size();
            sent_frames_++;
            sent_bytes_ += frame.payload.size();

        } else if (frame.opcode == WsFrame::OpCode::kText) {
            //! 文本帧：回复提示，仅接收二进制数据
            ws_srv_.send(token, "此服务仅接收二进制帧，请发送 ArrayBuffer");

        } else {
            //! 其他帧（Ping/Pong/Close 等）：忽略
        }
    }

    //! 定时器回调：构建统计帧，推送给所有客户端
    void onStatTimer()
    {
        //! 构建 JSON 统计信息
        std::string json = "{"
            "\"recv_frames\":" + std::to_string(recv_frames_) + ","
            "\"recv_bytes\":" + std::to_string(recv_bytes_) + ","
            "\"sent_frames\":" + std::to_string(sent_frames_) + ","
            "\"sent_bytes\":" + std::to_string(sent_bytes_) + ","
            "\"clients\":" + std::to_string(conns_.size()) +
        "}";

        //! 演示 WsServer::sendBinary() 的 vector<uint8_t> 版本
        //! 格式：4字节头 "STAT" + JSON 字符串字节
        std::vector<uint8_t> stat_data;
        stat_data.reserve(4 + json.size());
        stat_data.insert(stat_data.end(), kStatHeader, kStatHeader + 4);
        stat_data.insert(stat_data.end(), json.begin(), json.end());

        //! 向所有客户端推送统计帧
        for (const auto &token : conns_)
            ws_srv_.sendBinary(token, stat_data);
    }

  private:
    Loop *wp_loop_;
    WsServer ws_srv_;
    TimerEvent *stat_timer_;

    std::set<WsServer::ConnToken> conns_;  //! 所有连接

    //! 收发统计
    uint64_t recv_frames_ = 0;
    uint64_t recv_bytes_  = 0;
    uint64_t sent_frames_ = 0;
    uint64_t sent_bytes_  = 0;
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

    //! 创建 Echo 服务，挂载到 /ws/echo
    EchoService echo_srv(sp_loop);
    if (!echo_srv.initialize(&http_srv, "/ws/echo")) {
        LogErr("init echo service fail");
        return 0;
    }

    //! 添加 HTTP 请求处理（主页面）
    http_srv.use(
        [&](ContextSptr ctx, const NextFunc &next) {
            if (ctx->req().url.path == "/") {
                ctx->res().status_code = StatusCode::k200_OK;
                ctx->res().headers["Content-Type"] = "text/html; charset=utf-8";
                ctx->res().body = kEchoBinHtml;
                return;
            }
            next();
        }
    );

    //! 启动服务
    http_srv.start();
    echo_srv.start();

    //! Ctrl+C 退出
    sp_sig_event->setCallback(
        [&] (int) {
            echo_srv.stop();
            http_srv.stop();
            sp_loop->exitLoop();
        }
    );

    LogInfo("start, listen at %s", bind_addr.c_str());
    sp_loop->runLoop();
    LogInfo("stop");

    echo_srv.cleanup();
    http_srv.cleanup();

    LogInfo("exit");
    return 0;
}
