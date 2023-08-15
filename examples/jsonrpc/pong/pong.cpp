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
#include <tbox/network/tcp_server.h>
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

    SetScopeExitAction(
        [=] {
            delete sig_event;
            delete loop;
        }
    );

    network::TcpServer tcp_server(loop);
    jsonrpc::RawProto proto;
    jsonrpc::Rpc rpc(loop);

    rpc.initialize(&proto, 3);
    std::string srv_addr = "/tmp/ping_pong.sock";

    network::TcpServer::ConnToken curr_client_token;   //! 当前的客户端

    tcp_server.initialize(network::SockAddr::FromString(srv_addr), 2);
    tcp_server.setConnectedCallback([&] (network::TcpServer::ConnToken client_token) {
        tcp_server.disconnect(curr_client_token);
        curr_client_token = client_token;
    });
    tcp_server.setDisconnectedCallback([&] (network::TcpServer::ConnToken client_token) {
        curr_client_token.reset();
    });
    tcp_server.setReceiveCallback([&] (network::TcpServer::ConnToken client_token, network::Buffer &buff) {
        while (buff.readableSize() > 0) {
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

    proto.setSendCallback([&] (const void* data_ptr, size_t data_size) {
        tcp_server.send(curr_client_token, data_ptr, data_size);
    });

    tcp_server.start();

    rpc.registeService("ping", [&] (int id, const Json &js_params, int &errcode, Json &js_result) {
        int ping_count = 0;
        util::json::GetField(js_params, "count", ping_count);
        js_result = js_params;
        return true;
    });

    sig_event->initialize(SIGINT, event::Event::Mode::kPersist);
    sig_event->enable();

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
