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
 * TLS 版 echo server 示例
 * 用法：tls_echo_server <ip:port> --cert <cert_file> --key <key_file>
 * 必须指定证书文件和密钥文件
 */

#include <iostream>
#include <cstring>

#include <tbox/network/tcp_server.h>
#include <tbox/network/tls_config.h>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/signal_event.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;

void PrintUsage(const char *prog)
{
    cout << "Usage: " << prog << " <ip:port> --cert <cert_file> --key <key_file>" << endl
         << "Exp  : " << prog << " 0.0.0.0:12345 --cert server.crt --key server.key" << endl;
}

int main(int argc, char **argv)
{
    string bind_addr_str;
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

    if (bind_addr_str.empty() || cert_file.empty() || key_file.empty()) {
        PrintUsage(argv[0]);
        return 0;
    }

    LogOutput_Enable();

    SockAddr bind_addr = SockAddr::FromString(bind_addr_str);

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    TcpServer server(sp_loop);

    //! 设置 TLS 配置（必须在 initialize 之前调用）
    TlsConfig tls_config;
    tls_config.cert_file = cert_file;
    tls_config.key_file = key_file;
    tls_config.verify_peer = false;  //! 测试环境不验证 client 证书
    if (!server.setTlsConfig(tls_config)) {
        LogErr("set tls config fail, need network_tls module");
        return 0;
    }

    server.initialize(bind_addr, 1);
    //! 当收到数据时，直接往 client 指定对象发回去
    server.setReceiveCallback(
        [&server] (const TcpServer::ConnToken &client, Buffer &buff) {
            std::string text((const char*)buff.readableBegin(), buff.readableSize());
            LogInfo("len:%u, text:%s", text.size(), text.c_str());
            server.send(client, buff.readableBegin(), buff.readableSize());
            buff.hasReadAll();
        }, 0
    );
    server.start();

    //! 注册 ctrl+C 停止信号
    SignalEvent *sp_stop_ev = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_stop_ev] { delete sp_stop_ev; });
    sp_stop_ev->initialize(SIGINT, Event::Mode::kOneshot);
    sp_stop_ev->setCallback(
        [sp_loop, &server] (int) {
            server.stop();
            sp_loop->exitLoop();
        }
    );
    sp_stop_ev->enable();

    LogInfo("tls echo server running ...");

    if (bind_addr.type() == SockAddr::Type::kIPv4) {
        IPAddress ip;
        uint16_t port;
        bind_addr.get(ip, port);
        cout << "try command: .install/bin/examples/network/tcp_client/tls_echo_client <IP>:" << port << endl;
    }

    sp_loop->runLoop();
    LogInfo("tls echo server stopped");

    return 0;
}
