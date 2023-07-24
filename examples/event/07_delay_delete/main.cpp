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
#include <tbox/event/timer_event.h>

using namespace std;
using namespace tbox::event;

void PrintUsage(const char *process_name)
{
    cout << "Usage:" << process_name << " epoll" << endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    Loop* sp_loop = Loop::New(argv[1]);
    if (sp_loop == nullptr) {
        cout << "fail, exit" << endl;
        return 0;
    }

    auto timer = sp_loop->newTimerEvent();
    timer->initialize(chrono::milliseconds(10), Event::Mode::kPersist);
    timer->setCallback(
        [=] {
            sp_loop->runNext(
                [=] {
                    cout << "deleted" << endl;
                    delete timer;
                    sp_loop->exitLoop();
                }
            );
            cout << "timeout" << endl;
        }
    );
    timer->enable();

    sp_loop->runLoop(Loop::Mode::kForever);

    delete sp_loop;
    return 0;
}
