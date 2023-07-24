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
#include "semaphore.hpp"
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/timer_event.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::coroutine;

/**
 * 生产者 -- 消费者测试
 */
TEST(Semaphore, TwoRoutines_ProduceAndConsumer)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    Semaphore sem(sch, 0);

    int times = 200;
    int count = 0;
    //! 生产者
    auto routine1_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < times; ++i) {
            sem.release();
            sch.yield();
        }
    };

    //! 消费者
    auto routine2_entry = [&] (Scheduler &sch) {
        while (!sch.isCanceled()) {
            sem.acquire();
            ++count;
        }
    };

    sch.create(routine1_entry);
    sch.create(routine2_entry);

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    EXPECT_EQ(times, count);

    sch.cleanup();
}

/**
 * 用定时器定时通过sem释放资源，协程循环接收数据
 *
 * 主要测试主协程的loop事件与子协程之间是否存在冲突
 */
TEST(Semaphore, TimerProduceAndConsumer)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);
    Semaphore sem(sch, 0);

    int times = 10;
    int count = 0;

    //! 定时器，定时生产
    int index = 0;
    auto timer = sp_loop->newTimerEvent();
    timer->initialize(chrono::milliseconds(10), Event::Mode::kPersist);
    SetScopeExitAction([timer]{ delete timer;});
    timer->setCallback(
        [&] {
            if (index < times) {
                sem.release();
                ++index;
            } else {
                sp_loop->exitLoop();
            }
        }
    );
    timer->enable();

    //! 消费者
    auto routine2_entry = [&] (Scheduler &sch) {
        while (!sch.isCanceled()) {
            sem.acquire();
            ++count;
        }
    };

    sch.create(routine2_entry);

    sp_loop->runLoop();

    EXPECT_EQ(count, times);

    sch.cleanup();
}

