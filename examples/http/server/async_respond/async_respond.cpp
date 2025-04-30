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
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <thread>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/log/async_stdout_sink.h>
#include <tbox/event/signal_event.h>
#include <tbox/eventx/timer_pool.h>
#include <tbox/eventx/thread_pool.h>
#include <tbox/http/server/server.h>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::eventx;
using namespace tbox::http;
using namespace tbox::http::server;

int main(int argc, char **argv)
{
    std::string bind_addr = "0.0.0.0:12345";

    if (argc == 2) {
        bind_addr = argv[1];
    }

    LogOutput_Enable();

    LogInfo("enter");

    auto sp_loop = Loop::New();
    auto sp_sig_event = sp_loop->newSignalEvent();

    TimerPool timer_pool(sp_loop);
    ThreadPool thread_pool(sp_loop);

    thread_pool.initialize(1);

    SetScopeExitAction(
        [=] {
            delete sp_sig_event;
            delete sp_loop;
        }
    );

    sp_sig_event->initialize(SIGINT, Event::Mode::kPersist);
    sp_sig_event->enable();

    Server srv(sp_loop);
    if (!srv.initialize(network::SockAddr::FromString(bind_addr), 1)) {
        LogErr("init srv fail");
        return 0;
    }

    srv.start();
    //srv.setContextLogEnable(true);    //! 调试时需要看详细收发数据时可以打开

    //! 添加请求处理
    srv.use(
        [&](ContextSptr ctx, const NextFunc &next) {
            if (ctx->req().url.path == "/") {
                ctx->res().status_code = StatusCode::k200_OK;
                ctx->res().body = \
R"(
<head>
</head>
<body>
    <p> <a href="/1" target="_blank">nodelay</a> </p>
    <p> <a href="/2" target="_blank">delay 5 sec</a> </p>
    <p> <a href="/3" target="_blank">delay 0~10 sec</a> </p>
</body>
)";
            } else if (ctx->req().url.path == "/1") {
                ctx->res().status_code = StatusCode::k200_OK;
                ctx->res().body = ctx->req().url.path;

            } else if (ctx->req().url.path == "/2") {
                timer_pool.doAfter(std::chrono::seconds(5), [ctx] {
                    ctx->res().status_code = StatusCode::k200_OK;
                    ctx->res().body = ctx->req().url.path;
                });

            } else if (ctx->req().url.path == "/3") {
                struct Tmp {
                    int result;
                };
                auto tmp = std::make_shared<Tmp>();
                thread_pool.execute(
                    [tmp] {
                        //! 模拟随机的，不确定时长的阻塞性行为
                        auto wait_msec = ::rand() % 10000;
                        LogTrace("wait_msec: %d", wait_msec);
                        std::this_thread::sleep_for(std::chrono::milliseconds(wait_msec));
                        tmp->result = wait_msec;
                    },
                    [ctx, tmp] {
                        ctx->res().status_code = StatusCode::k200_OK;
                        ctx->res().body = "wait: " + std::to_string(tmp->result) + " msec";
                    }
                );
            }
        }
    );

    sp_sig_event->setCallback(
        [&] (int) {
            srv.stop();
            sp_loop->exitLoop();
        }
    );

    LogInfo("start");
    sp_loop->runLoop();
    LogInfo("stop");
    srv.cleanup();

    thread_pool.cleanup();
    timer_pool.cleanup();

    LogInfo("exit");
    return 0;
}
