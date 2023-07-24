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
 * 实现一个tcp的客户端
 * 它主动连接指定的服务端，在连接成功后，将终端输入的内容发送给服务端，将服务端的数据显示在终端上
 * 当连接被断开后，它再次尝试连接
 *
 * 本示例主要用于演示TcpConnector的使用方法
 */

#include <iostream>

#include <tbox/network/tcp_connector.h>
#include <tbox/network/tcp_connection.h>
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
    stdio.enable();

    TcpConnection *sp_curr = nullptr;

    TcpConnector connector(sp_loop);
    connector.initialize(bind_addr);
    //! 指定有Client连接上后该做的事务
    connector.setConnectedCallback(
        [&] (TcpConnection *new_conn) {
            //! (1) 指定Client将来断开时要做的事务
            new_conn->setDisconnectedCallback(
                [&, new_conn] {
                    sp_curr = nullptr;
                    connector.start();  //! 重新启动连接
                    sp_loop->runNext([new_conn] { delete new_conn; });
                }
            );
            sp_curr = new_conn;
            new_conn->bind(&stdio);     //! (2) TCP的数据往终端输出
            stdio.bind(new_conn);       //! (3) 终端上的输入往TCP输出
        }
    );

    //! 设置连接尝试次数，与多次尝试失败后的处理
    connector.setTryTimes(3);
    connector.setConnectFailCallback(
        [&] {
            //! 打印提示要开服务
            cout << "Connect server fail!" << endl
                 << "You should run command:" << endl;
            if (bind_addr.type() == SockAddr::Type::kIPv4) {
                IPAddress ip;
                uint16_t port;
                bind_addr.get(ip, port);
                cout << "  nc -l " << port << endl;
            } else {
                cout << "  nc -lU " << bind_addr.toString() << endl;
            }
            cout << "first." << endl;
            sp_loop->exitLoop();    //! (3) 退出事件循环
        }
    );

    connector.start();

    //! 注册ctrl+C停止信号
    SignalEvent *sp_stop_ev = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_stop_ev] { delete sp_stop_ev; });
    sp_stop_ev->initialize(SIGINT, Event::Mode::kOneshot);
    //! 指定ctrl+C时要做的事务
    sp_stop_ev->setCallback(
        [sp_loop, &sp_curr] (int) {
            if (sp_curr != nullptr) {
                sp_curr->disconnect();  //! (1) 主动断开连接
                delete sp_curr;         //! (2) 销毁Client对象。思考：为什么这里可以直接delete，而L51不可以？
            }
            sp_loop->exitLoop();    //! (3) 退出事件循环
        }
    );
    sp_stop_ev->enable();

    LogInfo("service runing ...");
    sp_loop->runLoop();
    LogInfo("service stoped");

    return 0;
}
