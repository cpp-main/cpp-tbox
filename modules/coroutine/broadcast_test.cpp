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
#include "broadcast.hpp"
#include <tbox/base/scope_exit.hpp>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::coroutine;

/**
 * 多个协程等待一个信号
 */
TEST(Broadcast, TwoRoutineUsingSharedValue)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    Broadcast bc(sch);
    int start_count = 0;
    int end_count = 0;

    auto wait_entry = [&] (Scheduler &sch) {
        ++start_count;
        bc.wait();
        if (sch.isCanceled())
            return;
        ++end_count;
    };

    auto post_entry = [&] (Scheduler &sch) {
        sch.yield();
        EXPECT_EQ(start_count, 3);  //! 检查是不是所有的协程都在等
        EXPECT_EQ(end_count, 0);
        bc.post();
    };

    sch.create(wait_entry);
    sch.create(wait_entry);
    sch.create(wait_entry);
    sch.create(post_entry);

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    EXPECT_EQ(end_count, 3);    //! 检查是不是所有的协程都等到了

    sch.cleanup();
}
