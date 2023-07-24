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
#include "condition.hpp"
#include "broadcast.hpp"
#include <tbox/base/scope_exit.hpp>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::coroutine;

/**
 * 一个协程同时等待两个协程
 */
TEST(Condition, WaitAll)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    Condition<string> cond(sch, Condition<string>::Logic::kAll);
    Broadcast bc(sch);

    int number = 0;

    auto wait_entry = [&] (Scheduler &sch) {
        cond.add("post1");
        cond.add("post2");
        cond.wait();    //! 要等 post_entry 对应的协程执行完才能执行
        EXPECT_EQ(number, 2);

        cond.add("post1");
        cond.add("post2");
        bc.post();
        cond.wait();    //! 再次等 post_entry 对应的协程执行完

        EXPECT_EQ(number, 4);
        (void)sch;
    };

    auto post_entry = [&] (Scheduler &sch) {
        number += 1;
        cond.post(sch.getName());

        bc.wait();  //! 等待广播信号

        number += 1;
        cond.post(sch.getName());
    };

    sch.create(wait_entry);
    sch.create(post_entry, true, "post1");
    sch.create(post_entry, true, "post2");

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();
}

//! 需要等待两个，结果只满足了一个
TEST(Condition, WaitAll_NotMeet)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    Condition<string> cond(sch, Condition<string>::Logic::kAll);
    Broadcast bc(sch);

    bool wait_done = false;

    auto wait_entry = [&] (Scheduler &sch) {
        cond.add("post1");
        cond.add("post2");
        cond.wait();
        wait_done = true;
        (void)sch;
    };

    auto post_entry = [&] (Scheduler &sch) {
        cond.post(sch.getName());
    };

    sch.create(wait_entry);
    sch.create(post_entry, true, "post1");

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    sch.cleanup();
}

TEST(Condition, WaitAny)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    Condition<string> cond(sch, Condition<string>::Logic::kAny);
    bool post_run = false;

    auto wait_entry = [&] (Scheduler &sch) {
        cond.add("post1");
        cond.add("post2");
        cond.wait();    //! 要等 post_entry 对应的协程执行完才能执行
        EXPECT_TRUE(post_run);
        (void)sch;
    };

    auto post_entry = [&] (Scheduler &sch) {
        post_run = true;
        cond.post(sch.getName());
    };

    sch.create(wait_entry);
    //! 只启动 post1，没有启动 post2
    sch.create(post_entry, true, "post1");

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    sch.cleanup();
}

