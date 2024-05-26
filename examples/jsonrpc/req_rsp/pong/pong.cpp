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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */

/**
 * 这是JsonRpc模块ping/pong示例中pong的一方
 *
 * 它绑定Unix Domain Sock: /tmp/ping_pong.sock，等待被连接。
 * 当收到ping请求后，将参数中的count提取出来作为结果直接回复。
 */

#include <tbox/base/log.h>  //! 打印日志
#include <tbox/base/log_output.h>   //! LogOutput_Enable()
#include <tbox/base/scope_exit.hpp> //! SetScopeExitAction()
#include <tbox/base/json.hpp>   //! 操作JSON对象用
#include <tbox/util/buffer.h>   //! 对Buffer的操作
#include <tbox/util/json.h>     //! util::json::GetField()
#include <tbox/event/loop.h>    //! 事件循环
#include <tbox/event/signal_event.h>    //! ctrl+c信号事件
#include <tbox/network/tcp_server.h>    //! TcpServer
#include <tbox/jsonrpc/protos/raw_stream_proto.h> //! jsonrpc::RawStreamProto
#include <tbox/jsonrpc/rpc.h>   //! jsonrpc::Rpc

using namespace tbox;

int main(int argc, char **argv)
{
    LogOutput_Enable();

    LogInfo("enter");

    auto loop = event::Loop::New();
    auto sig_event = loop->newSignalEvent();

    //! 设置退出时，要释放loop与sig_event
    SetScopeExitAction(
        [=] {
            delete sig_event;
            delete loop;
        }
    );

    network::TcpServer tcp_server(loop);
    jsonrpc::RawStreamProto proto;
    jsonrpc::Rpc rpc(loop);

    rpc.initialize(&proto, 3);
    std::string srv_addr = "/tmp/ping_pong.sock";

    network::TcpServer::ConnToken curr_client_token;   //! 当前的客户端

    tcp_server.initialize(network::SockAddr::FromString(srv_addr), 2);
    //! 设置接收到连接后的动作：保存curr_client_token
    tcp_server.setConnectedCallback([&] (network::TcpServer::ConnToken client_token) {
        tcp_server.disconnect(curr_client_token);
        curr_client_token = client_token;
    });
    //! 设置连接断开后的动作：清除curr_client_token
    tcp_server.setDisconnectedCallback([&] (network::TcpServer::ConnToken client_token) {
        curr_client_token.reset();
    });
    //! 设置接收到数据后的处理
    tcp_server.setReceiveCallback([&] (network::TcpServer::ConnToken client_token, network::Buffer &buff) {
        while (buff.readableSize() > 0) {
            //! 将buff中的数据交给proto进行解析
            auto ret = proto.onRecvData(buff.readableBegin(), buff.readableSize());
            if (ret > 0) {
                buff.hasRead(ret);
            } else if (ret < 0) {   //! 有错误
                tcp_server.disconnect(curr_client_token);
                curr_client_token.reset();
            } else
                break;
        }
    }, 0);

    //! 设置proto发送数据的方法
    proto.setSendCallback([&] (const void* data_ptr, size_t data_size) {
        tcp_server.send(curr_client_token, data_ptr, data_size);
    });

    tcp_server.start(); //! 启动tcp服务

    //! 注册ping的服务处理函数
    rpc.addService("ping", [&] (int id, const Json &js_params, int &errcode, Json &js_result) {
        int ping_count = 0;
        util::json::GetField(js_params, "count", ping_count);
        LogDbg("got ping_count: %d", ping_count);
        js_result = js_params;
        return true;    //! 表示在函数返回后立即发送回复
    });

    //! 设置程序安全退出条件
    sig_event->initialize(SIGINT, event::Event::Mode::kPersist);
    sig_event->enable();

    //! 设置程序退出动作
    sig_event->setCallback(
        [&] (int) {
            tcp_server.stop();
            loop->exitLoop();
        }
    );

    LogInfo("start");
    loop->runLoop();
    LogInfo("stop");

    rpc.cleanup();
    tcp_server.cleanup();
    return 0;
}
