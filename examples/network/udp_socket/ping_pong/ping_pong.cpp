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
 * 乒乓测试，ping 与 pong 相互发送数据
 *
 * 设定规则：
 *   - pong 在收到时，向 ping 发送一个字节；
 *   - ping 在收到时，向 pong 发送一个字节；
 * 周则复始。
 *
 * 在程序启动时，由 ping 首先发送第一个字节。
 */
#include <iostream>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/loop.h>
#include <tbox/network/udp_socket.h>

using namespace std;
using namespace tbox::event;
using namespace tbox::network;

int main()
{
    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    UdpSocket ping(sp_loop);
    UdpSocket pong(sp_loop);

    ping.bind(SockAddr::FromString("127.0.0.1:6670"));
    pong.bind(SockAddr::FromString("127.0.0.1:6671"));

    ping.connect(SockAddr::FromString("127.0.0.1:6671"));
    pong.connect(SockAddr::FromString("127.0.0.1:6670"));

    ping.setRecvCallback(
        [&ping] (const void *data_ptr, size_t data_size, const SockAddr &from) {
            char dummy = 0;
            ping.send(&dummy, 1);
        }
    );
    int count = 0;
    pong.setRecvCallback(
        [&pong, &count] (const void *data_ptr, size_t data_size, const SockAddr &from) {
            char dummy = 1;
            pong.send(&dummy, 1);
            ++count;
        }
    );

    ping.enable();
    pong.enable();

    char dummy = 0;
    ping.send(&dummy, 1);   //! 发球

    sp_loop->exitLoop(chrono::seconds(5));  //! 5秒后自动停止
    sp_loop->runLoop();

    cout << "count:" << count << endl;
    return 0;
}
