/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *   //  E A S Y  /  \/ \
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
/**
 * HTTPS 版 simple server 示例
 * 用法：https_simple <ip:port> --cert <cert_file> --key <key_file>
 * 必须指定证书文件和密钥文件
 */

#include <iostream>
#include <cstring>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/signal_event.h>
#include <tbox/http/server/server.h>
#include <tbox/network/tls_config.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::http;
using namespace tbox::http::server;
using namespace tbox::network;

void PrintUsage(const char *prog)
{
    cout << "Usage: " << prog << " <ip:port> --cert <cert_file> --key <key_file>" << endl
         << "Exp  : " << prog << " 0.0.0.0:12345 --cert server.crt --key server.key" << endl;
}

int main(int argc, char **argv)
{
    string bind_addr_str = "0.0.0.0:12345";
    string cert_file;
    string key_file;

    //! 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--cert") == 0 && i + 1 < argc) {
            cert_file = argv[++i];
        } else if (strcmp(argv[i], "--key") == 0 && i + 1 < argc) {
            key_file = argv[++i];
        } else if (argv[i][0] != '-') {
            bind_addr_str = argv[i];
        } else {
            cerr << "Error: invalid option `" << argv[i] << "'" << endl;
            PrintUsage(argv[0]);
            return 0;
        }
    }

    if (cert_file.empty() || key_file.empty()) {
        PrintUsage(argv[0]);
        return 0;
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

    Server srv(sp_loop);

    //! 设置 TLS 配置（必须在 initialize() 之前调用）
    TlsConfig tls_config;
    tls_config.cert_file = cert_file;
    tls_config.key_file = key_file;
    tls_config.verify_peer = false;  //! 测试环境不验证 client 证书
    if (!srv.setTlsConfig(tls_config)) {
        LogErr("set tls config fail, need network_tls module");
        return 0;
    }

    if (!srv.initialize(SockAddr::FromString(bind_addr_str), 1)) {
        LogErr("init srv fail");
        return 0;
    }

    srv.start();
    //srv.setContextLogEnable(true);    //! 调试时需要看详细收发数据时可以打开

    //! 添加请求处理
    srv.use(
        [&](ContextSptr ctx, const NextFunc &next) {
            ctx->res().status_code = StatusCode::k200_OK;
            ctx->res().body = "Hello HTTPS!";
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

    LogInfo("exit");
    return 0;
}
