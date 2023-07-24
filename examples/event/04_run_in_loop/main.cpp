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
#include <unistd.h>
#include <iostream>
#include <cassert>
#include <thread>
#include <string>
#include <tbox/event/loop.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

const int loop_times = 1000000;

void DoInLoop(Loop *wp_loop, int count)
{
    static int _count = 0;
    ++_count;

    assert(wp_loop->isInLoopThread());
    //cout << "DoInLoop(" << count << ")" << endl;
    if (_count == loop_times)
        cout << "DoInLoop: all received" << endl;
}

void ExitLoop(Loop* wp_loop)
{
    cout << "ExitLoop()" << endl;
    wp_loop->exitLoop(std::chrono::milliseconds(10));
}

void ThreadFunc1(Loop *wp_loop)
{
    assert(!wp_loop->isInLoopThread());
    cout << "ThreadFunc1, start" << endl;
    for (int i = 0; i < loop_times; ++i) {
        //cout << "ThreadFunc1, count:" << i << endl;
        wp_loop->runInLoop(bind(DoInLoop, wp_loop, i));
    }
    cout << "ThreadFunc1, exit" << endl;
    wp_loop->runInLoop(bind(ExitLoop, wp_loop));
}

void SayHello(int count)
{
    static int _count = 0;
    ++_count;

    if (_count == loop_times)
        cout << "SayHello: all received" << endl;
    //cout << "SayHello(" << count << ")" << endl;
}

void ThreadFunc2(Loop* wp_loop)
{
    cout << "ThreadFunc2, start" << endl;

    for (int i = 0; i < loop_times; ++i) {
        //cout << "ThreadFunc2, count:" << i << endl;
        wp_loop->runInLoop(bind(SayHello, i));
    }

    cout << "ThreadFunc2, exit" << endl;
}

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

    std::thread t1(bind(ThreadFunc1, sp_loop));
    std::thread t2(bind(ThreadFunc2, sp_loop));

    sp_loop->runLoop(Loop::Mode::kForever);

    t1.join();
    t2.join();

    delete sp_loop;
    return 0;
}
