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
#include "timer_pool.h"
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/base/scope_exit.hpp>

using namespace std;
using namespace std::chrono;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::eventx;

const int kAcceptableError = 10;

/**
 * 创建一个100ms的周期性定时任务
 * 在每次执行的时候，检查任务是否在 N * 100 ms 左右
 * 并且执行了10次。
 */
TEST(TimerPool, doEvery)
{
    Loop *sp_loop = event::Loop::New();
    TimerPool timer_pool(sp_loop);
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    auto start_time = steady_clock::now();
    int count = 0;
    TimerPool::TimerToken token;
    token = timer_pool.doEvery(milliseconds(100),
        [&] {
            ++count;
            auto d = steady_clock::now() - start_time;
            EXPECT_GT(d, milliseconds(count * 100 - kAcceptableError));
            EXPECT_LT(d, milliseconds(count * 100 + kAcceptableError));
        }
    );
    sp_loop->exitLoop(chrono::milliseconds(1010));
    sp_loop->runLoop();

    timer_pool.cleanup();
    sp_loop->cleanup();

    EXPECT_EQ(count, 10);
}

/**
 * 先创建一个 500ms 的单次执行任务
 * 在任务执行中，检查执行的时间范围是否在 490~510ms 之间
 */
TEST(TimerPool, doAfter)
{
    Loop *sp_loop = event::Loop::New();
    TimerPool timer_pool(sp_loop);
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    auto start_time = steady_clock::now();
    TimerPool::TimerToken token;
    bool is_run = false;
    token = timer_pool.doAfter(milliseconds(500),
        [&] {
            auto d = steady_clock::now() - start_time;
            EXPECT_GT(d, milliseconds(500 - kAcceptableError));
            EXPECT_LT(d, milliseconds(500 + kAcceptableError));
            is_run = true;
        }
    );
    sp_loop->exitLoop(chrono::milliseconds(1500));
    sp_loop->runLoop();

    timer_pool.cleanup();
    EXPECT_TRUE(is_run);
}

/**
 * 先创建一个 100ms 的定时任务。
 * 再创建一个 50ms 的定时器，在任务中取消上一个定时任务。
 * 观察 is_run 是否为 false
 */
TEST(TimerPool, cancel_inside_loop)
{
    Loop *sp_loop = event::Loop::New();
    TimerPool timer_pool(sp_loop);
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    bool is_run = false;
    auto token = timer_pool.doAfter(milliseconds(100),
        [&] {
            is_run = true;
        }
    );

    auto t = sp_loop->newTimerEvent();
    SetScopeExitAction([t]{ delete t;});
    t->initialize(chrono::milliseconds(50), Event::Mode::kOneshot);
    t->setCallback([&] { timer_pool.cancel(token); });
    t->enable();

    sp_loop->exitLoop(chrono::milliseconds(200));
    sp_loop->runLoop();

    timer_pool.cleanup();

    EXPECT_FALSE(is_run);
}

/**
 * 先创建一个 100ms 的定时任务。
 * 在执行 runLoop() 之前就 timer_pool.cancel()
 * 观察 is_run 是否为 false
 */
TEST(TimerPool, cancel_outside_loop)
{
    Loop *sp_loop = event::Loop::New();
    TimerPool timer_pool(sp_loop);
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    bool is_run = false;
    auto token = timer_pool.doAfter(milliseconds(100),
        [&] {
            is_run = true;
        }
    );

    timer_pool.cancel(token);
    sp_loop->exitLoop(chrono::milliseconds(200));
    sp_loop->runLoop();

    timer_pool.cleanup();

    EXPECT_FALSE(is_run);
}

/**
 * 创建一个1000ms后的任务。在该任务执行的时候检查执行时间是否在 990 ~ 1010 ms 之间
 * 在 1500ms 后停止
 */
TEST(TimerPool, doAt)
{
    Loop *sp_loop = event::Loop::New();
    TimerPool timer_pool(sp_loop);
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    auto start_time = system_clock::now();
    TimerPool::TimerToken token;
    bool is_run = false;
    token = timer_pool.doAt(start_time + milliseconds(1000),
        [&] {
            auto d = system_clock::now() - start_time;
            EXPECT_GT(d, milliseconds(1000 - kAcceptableError));
            EXPECT_LT(d, milliseconds(1000 + kAcceptableError));
            is_run = true;
        }
    );
    sp_loop->exitLoop(chrono::milliseconds(1500));
    sp_loop->runLoop();

    timer_pool.cleanup();

    EXPECT_TRUE(is_run);
}

/**
 * 创建一个周期性100ms的任务，每次任务执行的时候都检查执行的时间点是否在 N * 100ms 左右。
 * 再创建第二个单次510ms的任务，去取消第一个周期性任务。
 * 最后创建第三个单次1010ms的任务，停止 loop。
 */
TEST(TimerPool, all)
{
    Loop *sp_loop = event::Loop::New();
    TimerPool timer_pool(sp_loop);
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    auto start_time = steady_clock::now();

    int count = 0;
    TimerPool::TimerToken token = timer_pool.doEvery(milliseconds(100),
        [&] {
            auto d = steady_clock::now() - start_time;
            ++count;
            EXPECT_GT(d, milliseconds(count * 100 - kAcceptableError));
            EXPECT_LT(d, milliseconds(count * 100 + kAcceptableError));
        }
    );

    bool is_run = false;
    timer_pool.doAfter(milliseconds(510),
        [&] {
            auto d = steady_clock::now() - start_time;
            EXPECT_GT(d, milliseconds(510 - kAcceptableError));
            EXPECT_LT(d, milliseconds(510 + kAcceptableError));
            timer_pool.cancel(token);
            is_run = true;
        }
    );

    timer_pool.doAfter(milliseconds(1010),
        [&] {
            sp_loop->exitLoop();
        }
    );

    sp_loop->runLoop();

    auto d = steady_clock::now() - start_time;
    EXPECT_GT(d, milliseconds(1010 - kAcceptableError));
    EXPECT_LT(d, milliseconds(1010 + kAcceptableError));

    timer_pool.cleanup();

    EXPECT_EQ(count, 5);
    EXPECT_TRUE(is_run);
}
