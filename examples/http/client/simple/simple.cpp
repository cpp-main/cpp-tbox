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
#include <tbox/http/client/client.h>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::http;
using namespace tbox::http::client;

int main(int argc, char **argv)
{
    std::string server_addr = "127.0.0.1:12345";

    if (argc == 2) {
        server_addr = argv[1];
    }

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

    Client http_client(sp_loop);
    if (!http_client.initialize(network::SockAddr::FromString(server_addr))) {
        LogErr("init http_client fail");
        return 0;
    }

    http_client.setAutoReconnect(true);
    http_client.setRequestTimeout(std::chrono::seconds(10));
    //http_client.setContextLogEnable(true);    //! 调试时需要看详细收发数据时可以打开

    http_client.setConnectedCallback(
        [] {
            LogInfo("connected to server");
        }
    );
    http_client.setDisconnectedCallback(
        [] {
            LogInfo("disconnected from server");
        }
    );

    http_client.start();

    //! 简单 GET 请求
    http_client.request(Method::kGet, "/",
        [](const Respond &res) {
            LogInfo("GET / => status: %d, body: %s",
                    (int)res.status_code, res.body.c_str());
        });

    //! POST 请求
    http_client.request(Method::kPost, "/api/data",
        "{\"key\":\"value\"}",
        {{"Content-Type", "application/json"}},
        [](const Respond &res) {
            LogInfo("POST /api/data => status: %d",
                    (int)res.status_code);
        });

    //! 完整 Request 对象
    Request req;
    req.method = Method::kPut;
    req.http_ver = HttpVer::k1_1;
    req.url.path = "/api/update";
    req.headers["Content-Type"] = "application/json";
    req.body = "{\"id\":123}";
    http_client.request(req,
        [](const Respond &res) {
            LogInfo("PUT /api/update => status: %d",
                    (int)res.status_code);
        });

    sp_sig_event->setCallback(
        [&] (int) {
            http_client.stop();
            sp_loop->exitLoop();
        }
    );

    LogInfo("start");
    sp_loop->runLoop();
    LogInfo("stop");
    http_client.cleanup();

    LogInfo("exit");
    return 0;
}
