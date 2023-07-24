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
#include <gtest/gtest.h>
#include "udp_socket.h"

#include <tbox/base/scope_exit.hpp>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;

TEST(UdpSocket, echo)
{
    Loop *sp_loop = Loop::New();    
    SetScopeExitAction([sp_loop]{ delete sp_loop; });

    UdpSocket udp_client(sp_loop);
    UdpSocket udp_server(sp_loop);

    SockAddr server_addr = SockAddr::FromString("127.0.0.1:40000");
    udp_server.bind(server_addr);
    udp_server.setRecvCallback(
        [&udp_server](const void *data_ptr, size_t data_size, const SockAddr &from_addr) {
            udp_server.send(data_ptr, data_size, from_addr);
        }
    );

    int send_times = 0;
    TimerEvent *sp_timer = sp_loop->newTimerEvent();
    SetScopeExitAction([sp_timer]{ delete sp_timer; });

    sp_timer->initialize(std::chrono::milliseconds(10), Event::Mode::kPersist);
    sp_timer->setCallback(
        [&send_times, &udp_client, server_addr, sp_timer] {
            for (int i = 0; i < 100; ++i)
                udp_client.send("123456789", 10, server_addr);
            ++send_times;
            if (send_times == 100)
                sp_timer->disable();
        }
    );

    int recv_count = 0;
    udp_client.setRecvCallback(
        [&recv_count](const void *data_ptr, size_t data_size, const SockAddr &from_addr) {
            ASSERT_STREQ((const char *)data_ptr, "123456789");
            ++recv_count;
            (void)data_size;
            (void)from_addr;
        }
    );

    sp_loop->runInLoop(
        [sp_loop]{
            sp_loop->exitLoop(std::chrono::seconds(3));
        }
    );

    udp_server.enable();
    udp_client.enable();
    sp_timer->enable();

    sp_loop->runLoop();

    EXPECT_EQ(recv_count, 10000);
}

TEST(UdpSocket, echo_connect)
{
    Loop *sp_loop = Loop::New();    
    SetScopeExitAction([sp_loop]{ delete sp_loop; });

    SockAddr server_addr = SockAddr::FromString("127.0.0.1:40000");

    UdpSocket udp_client(sp_loop);
    UdpSocket udp_server(sp_loop);

    udp_server.bind(server_addr);
    udp_client.connect(server_addr);

    udp_server.setRecvCallback(
        [&udp_server](const void *data_ptr, size_t data_size, const SockAddr &addr) {
            udp_server.send(data_ptr, data_size, addr);
        }
    );

    int send_times = 0;
    TimerEvent *sp_timer = sp_loop->newTimerEvent();
    SetScopeExitAction([sp_timer]{ delete sp_timer; });

    sp_timer->initialize(std::chrono::milliseconds(10), Event::Mode::kPersist);
    sp_timer->setCallback(
        [&send_times, &udp_client, sp_timer] {
            for (int i = 0; i < 100; ++i)
                udp_client.send("123456789", 10);
            ++send_times;
            if (send_times == 100)
                sp_timer->disable();
        }
    );

    int recv_count = 0;
    udp_client.setRecvCallback(
        [&recv_count](const void *data_ptr, size_t data_size, const SockAddr &) {
            ASSERT_STREQ((const char *)data_ptr, "123456789");
            ++recv_count;
            (void)data_size;
        }
    );

    udp_server.enable();
    udp_client.enable();
    sp_timer->enable();

    sp_loop->exitLoop(std::chrono::seconds(3));
    sp_loop->runLoop();

    EXPECT_EQ(recv_count, 10000);
}
