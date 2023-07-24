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
#include "scheduler.h"
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/base/scope_exit.hpp>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::coroutine;

TEST(Scheduler, CreateTwoRoutineThenStop)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    int exec_count = 0;
    {
        Scheduler sch(sp_loop);
        auto entry = [&exec_count] (Scheduler &sch) {
            ++exec_count;
            (void)sch;
        };
        sch.create(entry, false, "test1");
        sch.create(entry, false, "test2");

        sp_loop->exitLoop(chrono::milliseconds(10));
        sp_loop->runLoop();
    }

    EXPECT_EQ(exec_count, 0);
}

TEST(Scheduler, CreateTwoRoutineStartOneThenStop)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    int exec_count = 0;
    {
        Scheduler sch(sp_loop);
        auto entry = [&exec_count] (Scheduler &sch) {
            ++exec_count;
            (void)sch;
        };
        sch.create(entry, true, "test1");
        sch.create(entry, false, "test2");

        sp_loop->exitLoop(chrono::milliseconds(10));
        sp_loop->runLoop();
    }

    EXPECT_EQ(exec_count, 1);
}

//! 测试在协程中是否能获取name与token
TEST(Scheduler, GetInfoInRoutine)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    string read_name;
    RoutineToken  read_token;

    auto entry = [&] (Scheduler &sch) {
        read_name = sch.getName();
        read_token = sch.getToken();
    };

    auto token1 = sch.create(entry, true, "test1");

    sp_loop->exitLoop(chrono::milliseconds(20));
    sp_loop->runLoop();

    EXPECT_EQ(read_name, "test1");
    EXPECT_TRUE(read_token.equal(token1));
}

//! 测试在协程中创建另一个协程，观察被创建的协程是否被执行
TEST(Scheduler, RoutineCreateAnotherRoutine)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    bool routine1_run = false;
    auto routine1_entry = [&] (Scheduler &sch) {
        routine1_run = true;
        (void)sch;
    };

    bool routine2_end = false;
    auto routine2_entry = [&] (Scheduler &sch) {
        sch.create(routine1_entry);
        sch.yield();
        routine2_end = true;
    };


    sch.create(routine2_entry);

    sp_loop->exitLoop(chrono::milliseconds(20));
    sp_loop->runLoop();

    EXPECT_TRUE(routine1_run);
    EXPECT_TRUE(routine2_end);
}

//! 测试 Scheduler的yield()
TEST(Scheduler, Yield)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    int routine1_count = 0;
    auto routine1_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < 20; ++i) {
            ++routine1_count;
            sch.yield();
        }
    };
    int routine2_count = 0;
    auto routine2_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < 10; ++i) {
            ++routine2_count;
            sch.yield();
        }
    };

    sch.create(routine1_entry);
    sch.create(routine2_entry);

    sp_loop->exitLoop(chrono::seconds(1));
    sp_loop->runLoop();

    EXPECT_EQ(routine1_count, 20);
    EXPECT_EQ(routine2_count, 10);
}

//! 测试 Scheduler::wait() 功能
TEST(Scheduler, Wait)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    bool routine_begin = false;
    bool routine_end = false;
    auto routine_entry = [&] (Scheduler &sch) {
        routine_begin = true;
        sch.wait();
        routine_end = true;
    };
    auto token = sch.create(routine_entry);

    //! 创建定时器，1秒后唤醒协程
    auto timer = sp_loop->newTimerEvent();
    SetScopeExitAction([timer]{ delete timer;});
    timer->initialize(chrono::milliseconds(10), Event::Mode::kOneshot);
    timer->setCallback(
        [&] {
            EXPECT_TRUE(routine_begin);
            EXPECT_FALSE(routine_end);
            sch.resume(token);
            sp_loop->exitLoop(chrono::milliseconds(10));    //! 不能直接停，要预留一点时间
        }
    );
    timer->enable();

    sp_loop->runLoop();

    EXPECT_TRUE(routine_begin);
    EXPECT_TRUE(routine_end);
}

//! 测试 Scheduler的yield()与cancel()功能
TEST(Scheduler, CancelRoutineByTimer)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    bool routine_stop = false;
    int  count = 0;
    auto routine_entry = [&] (Scheduler &sch) {
        while (true) {
            ++count;
            sch.yield();
            if (sch.isCanceled())
                break;
        }
        routine_stop = true;
    };
    auto token = sch.create(routine_entry);

    //! 创建定时器，1秒后取消协程
    auto timer = sp_loop->newTimerEvent();
    SetScopeExitAction([timer]{ delete timer;});
    timer->initialize(chrono::milliseconds(10), Event::Mode::kOneshot);
    timer->setCallback(
        [&] {
            sch.cancel(token);
            sp_loop->exitLoop(chrono::milliseconds(10));    //! 不能直接停，要预留一点时间
        }
    );
    timer->enable();

    sp_loop->runLoop();

    EXPECT_NE(count, 0);
    EXPECT_TRUE(routine_stop);
}

//! 测试 Scheduler的join()
TEST(Scheduler, JoinSubRoutine)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);
    bool sub_exit = false;
    int count = 0;
    int times = 10;
    auto sub_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < times; ++i) {
            sch.yield();
            ++count;
        }
        sub_exit = true;
    };

    bool main_exit = false;
    auto main_entry = [&] (Scheduler &sch) {
        sch.join(sch.create(sub_entry));    //! 创建子协程，并等待其结束

        EXPECT_TRUE(sub_exit);
        EXPECT_EQ(count, times);
        main_exit = true;
    };

    sch.create(main_entry);

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    EXPECT_TRUE(main_exit);

    sch.cleanup();
}

TEST(Scheduler, OneLoopTwoSchedule)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch1(sp_loop);
    Scheduler sch2(sp_loop);

    bool sch1_routine1_run = false;
    bool sch1_routine2_run = false;
    bool sch2_routine_run = false;

    sch1.create(
        [&](Scheduler &sch) {
            sch1_routine1_run = true;
            (void)sch;
        }
    );
    sch1.create(
        [&](Scheduler &sch) {
            sch1_routine2_run = true;
            (void)sch;
        }
    );
    sch2.create(
        [&](Scheduler &sch) {
            sch2_routine_run = true;
            (void)sch;
        }
    );

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    sch1.cleanup();
    sch2.cleanup();

    EXPECT_TRUE(sch1_routine1_run);
    EXPECT_TRUE(sch1_routine2_run);
    EXPECT_TRUE(sch2_routine_run);
}
