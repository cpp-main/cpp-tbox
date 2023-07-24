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

    TcpServer server(sp_loop);
    server.initialize(bind_addr, 1);
    //! 当收到数据时，直接往client指定对象发回去
    server.setReceiveCallback(
        [&server] (const TcpServer::ConnToken &client, Buffer &buff) {
            server.send(client, buff.readableBegin(), buff.readableSize());
            buff.hasReadAll();
        }, 0
    );
    server.start();

    //! 注册ctrl+C停止信号
    SignalEvent *sp_stop_ev = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_stop_ev] { delete sp_stop_ev; });
    sp_stop_ev->initialize(SIGINT, Event::Mode::kOneshot);
    //! 指定ctrl+C时要做的事务
    sp_stop_ev->setCallback(
        [sp_loop, &server] (int) {
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
