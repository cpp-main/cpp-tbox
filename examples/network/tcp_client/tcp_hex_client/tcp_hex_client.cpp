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
 * 它主动连接指定的服务端，在连接成功后，服务器发送什么就在终端上显示什么，终端上输入的内容将发送到服务器
 * 当连接被断开后，它再次尝试连接
 *
 * 本示例主要用于演示TcpClient的使用方法
 */

#include <iostream>

#include <tbox/network/tcp_client.h>
#include <tbox/network/stdio_stream.h>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/signal_event.h>
#include <tbox/util/string.h>

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

    SockAddr bind_addr = SockAddr::FromString(argv[1]);

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    StdioStream stdio(sp_loop);
    stdio.enable();

    TcpClient client(sp_loop);
    client.initialize(bind_addr);
    client.start();

    bool is_connected = false;
    bool is_prompt_print = false;

    auto print_prompt = [&] {
        cout << "> " << flush;
        is_prompt_print = true;
    };

    auto print_endline = [&] {
        if (is_prompt_print)
            cout << endl;
        is_prompt_print = false;
    };

    client.setReceiveCallback(
        [&] (Buffer &buff) {
            string hex_str = util::string::RawDataToHexStr(buff.readableBegin(), buff.readableSize());
            print_endline();
            cout << "recv " << buff.readableSize() << " : " << hex_str << endl;
            buff.hasReadAll();
        }
        , 0
    );

    client.setConnectedCallback(
        [&] {
            is_connected = true;
            print_endline();
            cout << "connected." << endl;
            print_prompt();
        }
    );

    client.setDisconnectedCallback(
        [&] {
            is_connected = false;
            print_endline();
            cout << "disconnected." << endl;
        }
    );

    stdio.setReceiveCallback(
        [&] (Buffer &buff) {
            if (is_connected) {
                if (buff.readableSize() > 1) {
                    try {
                        vector<uint8_t> raw_data;
                        std::string input_str(reinterpret_cast<const char*>(buff.readableBegin()), buff.readableSize() - 1);
                        util::string::HexStrToRawData(input_str, raw_data, "\t ");
                        client.send(raw_data.data(), raw_data.size());

                        string hex_str = util::string::RawDataToHexStr(raw_data.data(), raw_data.size());
                        cout << "send " << raw_data.size() << " : " << hex_str << endl;

                    } catch (const exception &e) {
                        cout << "Warn: input invalid: " << e.what() << endl;
                    }
                }
                print_prompt();
            } else {
                print_endline();
                cout << "not connect." << endl;
            }

            buff.hasReadAll();
        }
        , 0
    );

    //! 注册ctrl+C停止信号
    SignalEvent *sp_stop_ev = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_stop_ev] { delete sp_stop_ev; });
    sp_stop_ev->initialize(SIGINT, Event::Mode::kOneshot);
    //! 指定ctrl+C时要做的事务
    sp_stop_ev->setCallback(
        [sp_loop, &client] (int) {
            client.stop();
            sp_loop->exitLoop();    //! (3) 退出事件循环
        }
    );
    sp_stop_ev->enable();

    LogInfo("service runing ...");
    sp_loop->runLoop();
    LogInfo("service stoped");

    print_endline();

    client.cleanup();

    return 0;
}
