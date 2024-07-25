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
#include "channel.hpp"
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/timer_event.h>
#include <algorithm>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::coroutine;

/**
 * 生产者 -- 消费者测试
 */
TEST(Channel, TwoRoutines_ProduceAndConsumer)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    Channel<int> ch(sch);

    vector<int> send_vec;
    vector<int> recv_vec;

    int times = 200;
    for (int i = 0; i < times; ++i)
        send_vec.push_back(i);
    random_shuffle(send_vec.begin(), send_vec.end());

    //! 生产者，将 send_vec 中的数据逐一发送到 ch
    auto routine1_entry = [&] (Scheduler &sch) {
        for (int i : send_vec) {
            ch << i;
            sch.yield();
        }
    };

    //! 消费者，从 ch 中读取数据，存入到 recv_vec 中
    auto routine2_entry = [&] (Scheduler &sch) {
        while (!sch.isCanceled()) {
            int i = 0;
            if (ch >> i) {
                recv_vec.push_back(i);
            }
        }
    };

    sch.create(routine1_entry);
    sch.create(routine2_entry);

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    //! 检查发送的数据与接收到数据是否对应
    EXPECT_EQ(send_vec, recv_vec);

    sp_loop->cleanup();
}

/**
 * 一个协程向ch发送数据，两个协程从ch取数据来处理
 *
 * 测试多个协程同时向同一个Channel取数据的功能是否正常
 */
TEST(Channel, ThreeRoutines_OneProduceAndTwoConsumer)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    Channel<int> ch(sch);

    vector<int> send_vec;
    vector<int> recv_vec;

    int times = 200;
    for (int i = 0; i < times; ++i)
        send_vec.push_back(i);
    random_shuffle(send_vec.begin(), send_vec.end());

    //! 生产者，将 send_vec 中的数据逐一发送到 ch
    auto routine1_entry = [&] (Scheduler &sch) {
        for (int i : send_vec) {
            ch << i;
            //cout << sch.getName() << " << " << i << endl;
            sch.yield();
            if (sch.isCanceled())
                break;
        }
    };

    //! 消费者，从 ch 中读取数据，存入到 recv_vec 中
    auto routine2_entry = [&] (Scheduler &sch) {
        while (!sch.isCanceled()) {
            int i = 0;
            if (ch >> i) {
                recv_vec.push_back(i);
                //cout << sch.getName() << " >> " << i << endl;
            }
        }
    };

    sch.create(routine1_entry, true, "r1");
    sch.create(routine2_entry, true, "r2");
    sch.create(routine2_entry, true, "r3");

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    //! 检查发送的数据与接收到数据是否对应
    EXPECT_EQ(send_vec, recv_vec);
}

/**
 * 用定时器定时向ch发送数据，协程循环接收数据
 *
 * 主要测试主协程的loop事件与子协程之间是否存在冲突
 */
TEST(Channel, TimerProduceAndConsumer)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    Channel<int> ch(sch);

    vector<int> send_vec;
    vector<int> recv_vec;

    int times = 10;
    for (int i = 0; i < times; ++i)
        send_vec.push_back(i);
    random_shuffle(send_vec.begin(), send_vec.end());

    size_t index = 0;
    auto timer = sp_loop->newTimerEvent();
    SetScopeExitAction([timer]{ delete timer;});
    timer->initialize(chrono::milliseconds(10), Event::Mode::kPersist);
    timer->setCallback(
        [&] {
            if (index < send_vec.size()) {
                ch << send_vec.at(index);
                ++index;
            } else {
                sp_loop->exitLoop();
            }
        }
    );
    timer->enable();

    //! 消费者，从 ch 中读取数据，存入到 recv_vec 中
    auto routine2_entry = [&] (Scheduler &sch) {
        while (!sch.isCanceled()) {
            int i = 0;
            if (ch >> i) {
                recv_vec.push_back(i);
            }
        }
    };

    sch.create(routine2_entry);

    sp_loop->runLoop();

    //! 检查发送的数据与接收到数据是否对应
    EXPECT_EQ(send_vec, recv_vec);

    sch.cleanup();
    sp_loop->cleanup();
}

