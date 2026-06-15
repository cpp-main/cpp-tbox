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
 * WebSocket 聊天客户端示例
 *
 * 功能：
 * - 连接到 WebSocket 聊天服务器（chat 示例）
 * - 从标准输入读取文本行，发送为 WebSocket 文本帧
 * - 收到服务器消息打印到标准输出
 * - Ctrl+C 断开连接并退出
 *
 * 用法：
 *   ./chat_client <server_addr> <url_path>
 *   示例: ./chat_client 127.0.0.1:8080 /ws/chat-1
 */

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/signal_event.h>
#include <tbox/event/fd_event.h>
#include <tbox/network/sockaddr.h>
#include <tbox/websocket/client/client.h>

#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;
using namespace tbox::websocket;
using namespace tbox::websocket::client;

int main(int argc, char **argv)
{
    std::string server_addr = "127.0.0.1:8080";
    std::string url_path = "/ws/chat-1";

    if (argc >= 2)
        server_addr = argv[1];
    if (argc >= 3)
        url_path = argv[2];

    LogOutput_Enable();

    LogInfo("enter");

    auto sp_loop = Loop::New();
    auto sp_sig_event = sp_loop->newSignalEvent();

    //! 创建 stdin 读取事件（非阻塞读取标准输入）
    auto sp_stdin_event = sp_loop->newFdEvent();

    SetScopeExitAction(
        [=] {
            delete sp_stdin_event;
            delete sp_sig_event;
            delete sp_loop;
        }
    );

    //! 创建 WebSocket 客户端
    Client ws_client(sp_loop);
    if (!ws_client.initialize(SockAddr::FromString(server_addr), url_path)) {
        LogErr("init ws client fail");
        return 0;
    }

    //! 设置回调
    ws_client.setConnectedCallback([&] {
        LogInfo("connected to %s%s", server_addr.c_str(), url_path.c_str());
        std::cout << "== 已连接到 " << server_addr << url_path << " ==" << std::endl;
        std::cout << "请输入用户名（第一条消息为登录名）：" << std::endl;

        //! 启动 stdin 读取
        sp_stdin_event->initialize(STDIN_FILENO, FdEvent::kReadEvent, Event::Mode::kPersist);
        sp_stdin_event->setCallback([&](short) {
            std::string line;
            if (std::getline(std::cin, line)) {
                if (!line.empty()) {
                    ws_client.send(line);
                }
            } else {
                //! stdin 关闭（EOF），断开连接
                ws_client.close();
                sp_stdin_event->disable();
            }
        });
        sp_stdin_event->enable();
    });

    ws_client.setDisconnectedCallback([&] {
        LogInfo("disconnected");
        std::cout << "== 已断开连接 ==" << std::endl;
    });

    ws_client.setMessageCallback([&](const WsFrame &frame) {
        if (frame.opcode == WsFrame::OpCode::kText) {
            std::cout << frame.payload << std::endl;
        }
    });

    ws_client.setErrorCallback([&] {
        LogNotice("ws error");
        std::cout << "== 连接出错 ==" << std::endl;
    });

    //! 设置二次退避策略
    ws_client.setReconnectDelayCalcFunc([] (int fail_count) { return 1 << (std::min(4, fail_count)); });

    //! Ctrl+C 退出
    sp_sig_event->initialize(SIGINT, Event::Mode::kOneshot);
    sp_sig_event->enable();
    sp_sig_event->setCallback(
        [&] (int) {
            LogInfo("stopping...");
            ws_client.close();
            sp_loop->exitLoop();
        }
    );

    //! 启动连接
    if (!ws_client.start()) {
        LogErr("start ws client fail");
        return 0;
    }

    LogInfo("connecting to %s%s ...", server_addr.c_str(), url_path.c_str());

    sp_loop->runLoop();

    ws_client.stop();
    ws_client.cleanup();

    LogInfo("exit");
    return 0;
}
