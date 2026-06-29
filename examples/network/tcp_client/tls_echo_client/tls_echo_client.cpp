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
 * TLS 版 echo client 示例
 * 用法：tls_echo_client <ip:port> [--ca <ca_file>] [--hostname <hostname>]
 * 连接成功后，stdin 输入的内容会发送到 server，server 返回的数据会显示在 stdout
 */

#include <iostream>
#include <cstring>

#include <tbox/network/tcp_client.h>
#include <tbox/network/tls_config.h>
#include <tbox/network/stdio_stream.h>

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
    cout << "Usage: " << prog << " <ip:port> [--ca <ca_file>] [--hostname <hostname>]" << endl
         << "Exp  : " << prog << " 127.0.0.1:12345" << endl
         << "       " << prog << " 127.0.0.1:12345 --ca server.crt --hostname myserver" << endl;
}

int main(int argc, char **argv)
{
    string server_addr_str;
    string ca_file;
    string hostname = "127.0.0.1";

    //! 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--ca") == 0 && i + 1 < argc) {
            ca_file = argv[++i];
        } else if (strcmp(argv[i], "--hostname") == 0 && i + 1 < argc) {
            hostname = argv[++i];
        } else if (argv[i][0] != '-') {
            server_addr_str = argv[i];
        } else {
            cerr << "Error: invalid option `" << argv[i] << "'" << endl;
            PrintUsage(argv[0]);
            return 0;
        }
    }

    if (server_addr_str.empty()) {
        PrintUsage(argv[0]);
        return 0;
    }

    LogOutput_Enable();

    SockAddr server_addr = SockAddr::FromString(server_addr_str);

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    StdioStream stdio(sp_loop);
    stdio.initialize();

    TcpClient client(sp_loop);

    //! 设置 TLS 配置（必须在 initialize 之前调用）
    TlsConfig tls_config;
    tls_config.hostname = hostname;          //! SNI hostname
    if (!ca_file.empty()) {
        tls_config.ca_file = ca_file;
        tls_config.verify_peer = true;
    } else {
        tls_config.verify_peer = false;       //! 未指定 CA 证书时，不做对端验证
    }
    if (!client.setTlsConfig(tls_config)) {
        LogErr("set tls config fail, need network_tls module");
        return 0;
    }

    client.initialize(server_addr);

    //! 连接成功后，绑定 stdio 和 client 的双向数据流
    client.setConnectedCallback(
        [&client, &stdio] {
            cout << "connected!" << endl;
            stdio.enable();                //! 连接成功后才启用 stdin 读取
            client.bind(&stdio);           //! server 的数据往终端输出
            stdio.bind(&client);           //! 终端上的输入往 server 输出
        }
    );

    client.setDisconnectedCallback(
        [&client, &stdio] {
            cout << "disconnected!" << endl;
            stdio.unbind();
            client.unbind();
        }
    );

    client.start();

    //! 注册 ctrl+C 停止信号
    SignalEvent *sp_stop_ev = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_stop_ev] { delete sp_stop_ev; });
    sp_stop_ev->initialize(SIGINT, Event::Mode::kOneshot);
    sp_stop_ev->setCallback(
        [sp_loop, &client] (int) {
            client.stop();
            sp_loop->exitLoop();
        }
    );
    sp_stop_ev->enable();

    LogInfo("tls echo client running ...");
    sp_loop->runLoop();
    LogInfo("tls echo client stopped");

    return 0;
}
