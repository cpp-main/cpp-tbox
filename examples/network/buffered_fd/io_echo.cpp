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
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>
#include <tbox/network/buffered_fd.h>

using namespace std;
using namespace tbox;

//! 一个终端的回显示例
int main()
{
    auto sp_loop = event::Loop::New();

    auto sp_stdin = new network::BufferedFd(sp_loop);
    auto sp_stdout = new network::BufferedFd(sp_loop);

    sp_stdin->initialize(STDIN_FILENO, network::BufferedFd::kReadOnly);
    sp_stdout->initialize(STDOUT_FILENO, network::BufferedFd::kWriteOnly);

    //! 将sp_stdin收到的数据转发给sp_stdout，实现重定向
    sp_stdin->bind(sp_stdout);

    sp_stdin->enable();
    sp_stdout->enable();

    auto sp_exit = sp_loop->newSignalEvent();
    sp_exit->initialize(SIGINT, event::Event::Mode::kOneshot);
    sp_exit->setCallback(
        [=] (int) {
            cout << "Info: Exit Loop" << endl;
            sp_loop->exitLoop();
        }
    );
    sp_exit->enable();

    cout << "Info: Start Loop" << endl;
    sp_loop->runLoop();
    cout << "Info: End Loop" << endl;

    delete sp_exit;
    delete sp_stdout;
    delete sp_stdin;
    delete sp_loop;
    return 0;
}
