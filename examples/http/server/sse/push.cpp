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
 * SSE 定时推送示例
 *
 * 功能：
 * - 创建 HTTP 服务器，挂载 SSE 服务到 /sse/events
 * - 每 5 秒向所有 SSE 客户端推送当前时间事件
 * - SSE 连接自动心跳（每 15 秒发送注释行）
 * - HTTP 主页提供浏览器 EventSource 客户端代码
 * - Ctrl+C 优雅退出
 *
 * 用法：
 *   ./sse_push [bind_addr]
 *   示例: ./sse_push 0.0.0.0:8080
 *   浏览器访问: http://127.0.0.1:8080/
 */

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/signal_event.h>
#include <tbox/event/timer_event.h>
#include <tbox/http/server/server.h>
#include <tbox/http/server/sse/sse_server.h>

#include <string>
#include <ctime>
#include <chrono>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::http;
using namespace tbox::http::server;
using namespace tbox::http::sse;

//! 获取当前时间字符串
static std::string getTimeString()
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t_now));
    return buf;
}

//! HTML 主页：包含 EventSource 客户端 JavaScript
static const std::string kIndexHtml =
    "<!DOCTYPE html>\n"
    "<html><head><title>SSE Push Demo</title></head>\n"
    "<body>\n"
    "<h1>SSE Push Demo</h1>\n"
    "<div id='events'></div>\n"
    "<script>\n"
    "var es = new EventSource('/sse/events');\n"
    "es.onmessage = function(e) {\n"
    "  var div = document.getElementById('events');\n"
    "  div.innerHTML = e.data + '<br>' + div.innerHTML;\n"
    "};\n"
    "es.addEventListener('tick', function(e) {\n"
    "  var div = document.getElementById('events');\n"
    "  div.innerHTML = '[tick] ' + e.data + '<br>' + div.innerHTML;\n"
    "});\n"
    "es.onerror = function() {\n"
    "  console.log('SSE connection error, browser will auto-reconnect');\n"
    "};\n"
    "</script>\n"
    "</body></html>\n";

int main(int argc, char **argv)
{
    std::string bind_addr = "0.0.0.0:8080";

    if (argc == 2)
        bind_addr = argv[1];

    LogOutput_Enable();

    LogInfo("enter");

    auto sp_loop = Loop::New();
    auto sp_sig_event = sp_loop->newSignalEvent();

    //! 心跳定时器
    auto sp_push_timer = sp_loop->newTimerEvent();

    SetScopeExitAction(
        [=] {
            delete sp_push_timer;
            delete sp_sig_event;
            delete sp_loop;
        }
    );

    //! 创建 HTTP 服务器
    Server http_srv(sp_loop);
    if (!http_srv.initialize(network::SockAddr::FromString(bind_addr), 1)) {
        LogErr("init http server fail");
        return 0;
    }
    //http_srv.setContextLogEnable(true);

    //! 创建 SSE 服务，挂载到 /sse/events
    SseServer sse_srv(sp_loop);
    if (!sse_srv.initialize(&http_srv, "/sse/events")) {
        LogErr("init sse server fail");
        return 0;
    }
    //sse_srv.setContextLogEnable(true);

    //! 设置自动心跳（每 15 秒发送注释行，保持连接活跃）
    sse_srv.setHeartbeatInterval(std::chrono::seconds(15));

    //! 设置 SSE 连接回调
    sse_srv.setConnectedCallback([&](const SseServer::ConnToken &token) {
        auto addr = sse_srv.peerAddr(token);
        LogInfo("sse client connected from %s", addr.toString().c_str());

        //! 向新连接发送欢迎消息
        sse_srv.send(token, "Welcome! SSE connection established.");
    });

    sse_srv.setDisconnectedCallback([](const SseServer::ConnToken &token) {
        LogInfo("sse client disconnected");
    });

    //! 定时推送：每 5 秒向所有客户端推送当前时间
    int event_id = 0;
    sp_push_timer->initialize(std::chrono::seconds(5), Event::Mode::kPersist);
    sp_push_timer->setCallback([&] {
        //! 构造 SSE 事件
        SseEvent evt;
        evt.id = std::to_string(++event_id);
        evt.event = "tick";
        evt.data = "{\"time\":\"" + getTimeString() + "\",\"id\":" + std::to_string(event_id) + "}";

        //! 向所有客户端推送
        sse_srv.sendToAll(evt);
        LogDbg("push event id:%d to clients", event_id);
    });

    //! 添加 HTTP 主页处理
    http_srv.use(
        [&](ContextSptr ctx, const NextFunc &next) {
            if (ctx->req().url.path == "/") {
                ctx->res().status_code = StatusCode::k200_OK;
                ctx->res().headers["Content-Type"] = "text/html; charset=utf-8";
                ctx->res().body = kIndexHtml;
                return;
            }
            next();
        }
    );

    //! 启动服务
    http_srv.start();
    sse_srv.start();
    sp_push_timer->enable();

    //! Ctrl+C 退出
    sp_sig_event->initialize(SIGINT, Event::Mode::kPersist);
    sp_sig_event->enable();
    sp_sig_event->setCallback(
        [&] (int) {
            LogInfo("stopping...");
            sp_push_timer->disable();
            sse_srv.stop();
            http_srv.stop();
            sp_loop->exitLoop();
        }
    );

    LogInfo("start, listen at %s", bind_addr.c_str());
    sp_loop->runLoop();
    LogInfo("stop");

    sse_srv.cleanup();
    http_srv.cleanup();

    LogInfo("exit");
    return 0;
}
