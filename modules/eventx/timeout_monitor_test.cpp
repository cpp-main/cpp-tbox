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
#include "timeout_monitor.hpp"
#include <tbox/base/scope_exit.hpp>

namespace tbox {
namespace eventx {
namespace {

using namespace event;
using namespace std::chrono;

TEST(TimeoutMonitor, Basic)
{
    auto sp_loop = Loop::New();
    SetScopeExitAction([=] {delete sp_loop;});

    TimeoutMonitor<int> tm(sp_loop);
    tm.initialize(milliseconds(100), 10);

    auto start_time = steady_clock::now();

    bool run = false;
    tm.setCallback([&] (int value) {
        EXPECT_EQ(value, 100);

        auto d = steady_clock::now() - start_time;
        EXPECT_GT(d, milliseconds(900));
        EXPECT_LT(d, milliseconds(1100));
        run = true;
    });

    tm.add(100);
    sp_loop->exitLoop(milliseconds(1200));
    sp_loop->runLoop();

    EXPECT_TRUE(run);
}

//! 测试中途clear()的操作，观察有没有被误触发
TEST(TimeoutMonitor, Clear)
{
    auto sp_loop = Loop::New();
    auto sp_timer = sp_loop->newTimerEvent();
    SetScopeExitAction(
      [=] {
        delete sp_loop;
        delete sp_timer;
      }
    );
    sp_timer->initialize(std::chrono::milliseconds(25), Event::Mode::kOneshot);

    TimeoutMonitor<int> tm(sp_loop);
    tm.initialize(milliseconds(10), 3);

    sp_timer->setCallback([&] { tm.clear(); });
    sp_timer->enable();

    bool run = false;
    tm.setCallback([&] (int value) {
        run = true;
    });

    tm.add(100);
    tm.add(101);

    sp_loop->exitLoop(milliseconds(120));
    sp_loop->runLoop();

    EXPECT_FALSE(run);
}

}
}
}
