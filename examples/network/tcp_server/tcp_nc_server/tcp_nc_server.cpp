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
/**
 * 实现一个tcp的echo服务，对方发送什么就回复什么
 */

#include <iostream>

#include <tbox/network/tcp_server.h>
#include <tbox/network/stdio_stream.h>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/signal_event.h>

#include <set>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;

void PrintUsage(const char *prog)
{
    cout << "Usage: " << prog << " <ip:port|localpath>" << endl
         << "Exp  : " << prog << " 127.0.0.1:12345" << endl
         << "       " << prog << " /tmp/test.sock" << endl;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    LogOutput_Enable();

    SockAddr bind_addr = SockAddr::FromString(argv[1]);

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    StdioStream stdio(sp_loop);
    stdio.initialize();
    stdio.enable();

    TcpServer server(sp_loop);
    server.initialize(bind_addr, 1);

    server.start();

    //! 定义TcpServer::Client的less函数子，以实现set<T>容器
    struct ClientLess {
        bool operator () (const TcpServer::ConnToken &lhs,
                          const TcpServer::ConnToken &rhs) const {
            return lhs.less(rhs);
        }
    };
    set<TcpServer::ConnToken, ClientLess> clients;

    //! 收到连接时，将client存入到clients中
    server.setConnectedCallback(
        [&clients] (const TcpServer::ConnToken &client) {
            clients.insert(client);
        }
    );
    //! 当连接断开时，将client从clients中移除
    server.setDisconnectedCallback(
        [&clients] (const TcpServer::ConnToken &client) {
            clients.erase(client);
        }
    );
    //! 收到tcp数据时，通过stdio发送
    server.setReceiveCallback(
        [&stdio] (const TcpServer::ConnToken &client, Buffer &buff) {
            stdio.send(buff.readableBegin(), buff.readableSize());
            buff.hasReadAll();
        }, 0
    );
    //! 收到stdio数据时，通过往每一个tcp发送
    stdio.setReceiveCallback(
        [&server, &clients] (Buffer &buff) {
            for (const auto &client : clients)
                server.send(client, buff.readableBegin(), buff.readableSize());
            buff.hasReadAll();
        }, 0
    );

    //! 注册ctrl+C停止信号
    SignalEvent *sp_stop_ev = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_stop_ev] { delete sp_stop_ev; });
    sp_stop_ev->initialize(SIGINT, Event::Mode::kOneshot);
    //! 指定ctrl+C时要做的事务
    sp_stop_ev->setCallback(
        [sp_loop, &server, &clients] (int) {
            clients.clear();
            server.stop();
            sp_loop->exitLoop();    //! (3) 退出事件循环
        }
    );
    sp_stop_ev->enable();

    LogInfo("service runing ...");

    //! 打印提示
    if (bind_addr.type() == SockAddr::Type::kIPv4) {
        IPAddress ip;
        uint16_t port;
        bind_addr.get(ip, port);
        cout << "try command: nc " << ip.toString() << ' ' << port << endl;
    } else {
        cout << "try command: nc -U " << bind_addr.toString() << endl;
    }

    sp_loop->runLoop();
    LogInfo("service stoped");

    return 0;
}
