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
#include <iostream>
#include <tbox/base/scope_exit.hpp>
#include <tbox/network/udp_socket.h>
#include <tbox/event/loop.h>

using namespace std;
using namespace tbox::network;
using namespace tbox::event;

int main()
{
    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop; });
    UdpSocket recv(sp_loop);
    recv.bind(SockAddr::FromString("127.0.0.1:6666"));
    recv.setRecvCallback(
        [] (const void *data_ptr, size_t data_size, const SockAddr &from) {
            const char *str = (const char *)data_ptr;
            cout << "recv from " << from.toString() << " : " << str << endl;
        }
    );
    recv.enable();

    cout << "start" << endl;
    sp_loop->runLoop();
    cout << "exit" << endl;

    return 0;
}
