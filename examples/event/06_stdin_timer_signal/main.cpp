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
#include <signal.h>
#include <iostream>
#include <string>
#include <tbox/event/loop.h>
#include <tbox/event/fd_event.h>
#include <tbox/event/timer_event.h>
#include <tbox/event/signal_event.h>
#include <tbox/event/stat.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void TimerCallback()
{
    cout << "." << flush;
}

void TimerCallback2()
{
    cout << "." << endl << flush;
}

void OneshotTimerCallback()
{
    cout << "#" << flush;
}

void printStat(Loop *wp_loop)
{
    Stat stat = wp_loop->getStat();
    cout << stat;
}

void StdinReadCallback(short events, Loop* wp_loop, TimerEvent* wp_timer)
{
    if (events & FdEvent::kReadEvent) {
        char buff[32];
        size_t rsize = read(STDIN_FILENO, buff, sizeof(buff));
        buff[rsize - 1] = '\0';
        string cmd(buff);

        if (cmd == "quit") {
            wp_loop->exitLoop(std::chrono::seconds::zero());
        } else if (cmd == "start") {
            wp_timer->enable();
        } else if (cmd == "stop") {
            wp_timer->disable();
        } else if (cmd == "init") {
            wp_timer->initialize(std::chrono::milliseconds(10), Event::Mode::kPersist);
        } else if (cmd == "cb2") {
            wp_timer->setCallback(TimerCallback2);
        } else if (cmd == "cb1") {
            wp_timer->setCallback(TimerCallback);
        } else if (cmd == "stat") {
            printStat(wp_loop);
        } else {
            cout << "Unknown command" << endl;
        }
    }
}

void IntSignalCallback(Loop* wp_loop)
{
    cout << "got signal" << endl;
    wp_loop->exitLoop(std::chrono::seconds(3));
    cout << "exit after 3 sec" << endl;
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


    TimerEvent* sp_timer = sp_loop->newTimerEvent();
    sp_timer->initialize(std::chrono::seconds(1), Event::Mode::kPersist);
    sp_timer->setCallback(TimerCallback);
    sp_timer->enable();

    TimerEvent* sp_timer_1 = sp_loop->newTimerEvent();
    sp_timer_1->initialize(std::chrono::seconds(5), Event::Mode::kOneshot);
    sp_timer_1->setCallback(OneshotTimerCallback);
    sp_timer_1->enable();

    FdEvent* sp_stdin = sp_loop->newFdEvent();
    sp_stdin->initialize(STDIN_FILENO, FdEvent::kReadEvent, Event::Mode::kPersist);
    sp_stdin->setCallback(bind(StdinReadCallback, std::placeholders::_1, sp_loop, sp_timer));
    sp_stdin->enable();

    SignalEvent* sp_sig_int = sp_loop->newSignalEvent();
    sp_sig_int->initialize(SIGINT, Event::Mode::kPersist);
    sp_sig_int->setCallback(bind(IntSignalCallback, sp_loop));
    sp_sig_int->enable();

    cout << "commands:" << endl
         << "start -- start timer" << endl
         << "stop  -- stop timer" << endl
         << "init  -- reinitialize timer" << endl
         << "cb1   -- set callback function" << endl
         << "cb2   -- set another callback function" << endl
         << "quit  -- exit test" << endl
         << "stat  -- print stat" << endl
         << "Press Ctrl+C to exit" << endl;

    sp_loop->runLoop(Loop::Mode::kForever);

    delete sp_sig_int;
    delete sp_stdin;
    delete sp_timer_1;
    delete sp_timer;
    delete sp_loop;
    return 0;
}
