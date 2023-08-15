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
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/json.hpp>
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>
#include <tbox/network/tcp_client.h>
#include <tbox/network/buffer.h>
#include <tbox/util/json.h>
#include <tbox/jsonrpc/raw_proto.h>
#include <tbox/jsonrpc/rpc.h>

using namespace tbox;

int main(int argc, char **argv)
{
    LogOutput_Enable();

    LogInfo("enter");

    auto loop = event::Loop::New();
    auto sig_event = loop->newSignalEvent();
    auto timer = loop->newTimerEvent();

    SetScopeExitAction(
        [=] {
            delete timer;
            delete sig_event;
            delete loop;
        }
    );

    network::TcpClient tcp_client(loop);
    jsonrpc::RawProto proto;
    jsonrpc::Rpc rpc(loop);

    rpc.initialize(&proto, 3);
    std::string srv_addr = "/tmp/ping_pong.sock";

    tcp_client.initialize(network::SockAddr::FromString(srv_addr));
    tcp_client.setReceiveCallback([&] (network::Buffer &buff) {
        while (buff.readableSize() > 0) {
            auto ret = proto.onRecvData(buff.readableBegin(), buff.readableSize());
            if (ret > 0) {
                buff.hasRead(ret);
            } else if (ret < 0) {   //! 有错误
                tcp_client.stop();
                tcp_client.start();
            } else
                break;
        }
    }, 0);

    proto.setSendCallback([&] (const void* data_ptr, size_t data_size) {
        tcp_client.send(data_ptr, data_size);
    });

    tcp_client.start();

    int ping_count = 0;
    std::function<void()> send_ping;
    send_ping = [&] {
        ++ping_count;
        Json js_params = {{"count", ping_count}};

        rpc.request("ping", js_params,
            [&] (int errcode, const Json &js_result) {
                int pong_count = 0;
                util::json::GetField(js_result, "count", pong_count);
                if (errcode == 0) {
                    LogDbg("got pong: %d", pong_count);
                    send_ping();
                } else
                    LogNotice("got erro: %d", errcode);
            });
        LogDbg("send ping: %d", ping_count);
    };

    tcp_client.setConnectedCallback(send_ping);

    sig_event->initialize(SIGINT, event::Event::Mode::kPersist);
    sig_event->enable();

    sig_event->setCallback(
        [&] (int) {
            tcp_client.stop();
            timer->disable();
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
