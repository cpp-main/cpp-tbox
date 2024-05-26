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
 * 这是JsonRpc模块ping/pong示例中ping的一方
 *
 * 它主动去连接/tmp/ping_pong.sock，连接成功后就会发送ping请求，并在请
 * 求中附带一个count参数。该count参数在每次请求之前都会自增1。
 * pong端收到ping请求后，将count作为结果回复该请求。
 * ping端收到回复后，就立即发送下一个ping请求。
 * 如此周而复始，直至与pong端断开，或者接收到了SIGINT停止信号
 */

#include <tbox/base/log.h>  //! 打印日志
#include <tbox/base/log_output.h>   //! 使能日志输出
#include <tbox/base/scope_exit.hpp> //! 使用 SetScopeExitAction()
#include <tbox/base/json.hpp>   //! 操作JSON对象用
#include <tbox/util/buffer.h>   //! 对Buffer进行操作
#include <tbox/event/loop.h>    //! 事件循环
#include <tbox/event/signal_event.h>    //! ctrl+c信号事件
#include <tbox/network/tcp_client.h>    //! 导入TcpClient模块
#include <tbox/util/json.h>     //! 使用JSON操作的辅助函数 GetField()
#include <tbox/jsonrpc/protos/raw_stream_proto.h>   //! 导入 jsonrpc::RawStreamProto
#include <tbox/jsonrpc/rpc.h>   //! 导入 jsonrpc::Rpc

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

    network::TcpClient tcp_client(loop);
    jsonrpc::RawStreamProto proto;
    jsonrpc::Rpc rpc(loop);

    rpc.initialize(&proto, 3);
    std::string srv_addr = "/tmp/ping_pong.sock";

    tcp_client.initialize(network::SockAddr::FromString(srv_addr));
    //! 设置接收到数据后的处理
    tcp_client.setReceiveCallback([&] (network::Buffer &buff) {
        while (buff.readableSize() > 0) {
            //! 将buff中的数据交给proto进行解析
            auto ret = proto.onRecvData(buff.readableBegin(), buff.readableSize());
            if (ret > 0) {  //! 正常解析
                buff.hasRead(ret);
            } else if (ret < 0) {   //! 有错误
                tcp_client.stop();
                tcp_client.start();
            } else
                break;
        }
    }, 0);

    //! 设置proto发送数据的方法
    proto.setSendCallback([&] (const void* data_ptr, size_t data_size) {
        tcp_client.send(data_ptr, data_size);   //! 将数据交给tcp_client去发送
    });

    tcp_client.start();

    int ping_count = 0;
    std::function<void()> send_ping;
    //! 定义ping的动作
    send_ping = [&] {
        ++ping_count;
        Json js_params = {{"count", ping_count}};   //! 组装请求参数

        //! 发送ping消息
        rpc.notify("ping", js_params);
        LogDbg("send ping: %d", ping_count);
    };

    //! 定义收到pong的动作
    rpc.addService("pong", [&] (int id, const Json &js_params, int &, Json &) {
        int pong_count = 0;
        util::json::GetField(js_params, "count", pong_count);
        send_ping();
        return false;
    });

    //! 设置一旦tcp_client连接上就进行ping动作
    tcp_client.setConnectedCallback(send_ping);

    //! 设置程序安全退出条件
    sig_event->initialize(SIGINT, event::Event::Mode::kPersist);
    sig_event->enable();

    //! 设置程序退出动作
    sig_event->setCallback(
        [&] (int) {
            tcp_client.stop();
            loop->exitLoop();
        }
    );

    LogInfo("start");
    loop->runLoop();
    LogInfo("stop");

    rpc.cleanup();
    tcp_client.cleanup();
    return 0;
}
