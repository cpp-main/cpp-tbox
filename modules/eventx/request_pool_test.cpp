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
#include "request_pool.hpp"
#include <tbox/base/scope_exit.hpp>

namespace tbox {
namespace eventx {
namespace {

using namespace event;
using namespace std::chrono;

TEST(RequestPool, Timeout)
{
    auto sp_loop = Loop::New();
    SetScopeExitAction([=] {delete sp_loop;});

    RequestPool<int> rp(sp_loop);
    rp.initialize(milliseconds(100), 10);

    auto start_time = steady_clock::now();

    rp.newRequest(new int(123));
    bool run = false;
    rp.setTimeoutAction([&] (int *p) {
        EXPECT_EQ(*p, 123);

        auto d = steady_clock::now() - start_time;
        EXPECT_GT(d, milliseconds(900));
        EXPECT_LT(d, milliseconds(1100));
        run = true;
    });

    sp_loop->exitLoop(milliseconds(1200));
    sp_loop->runLoop();

    EXPECT_TRUE(run);
}

TEST(RequestPool, NotTimeout)
{
    auto sp_loop = Loop::New();
    SetScopeExitAction([=] {delete sp_loop;});

    RequestPool<int> rp(sp_loop);
    rp.initialize(milliseconds(100), 10);

    auto token = rp.newRequest(new int(123));
    bool run = false;
    rp.setTimeoutAction([&] (int *p) { run = true; (void)p; });

    auto req = rp.removeRequest(token);
    EXPECT_TRUE(req != nullptr);
    EXPECT_EQ(*req, 123);
    delete req;

    sp_loop->exitLoop(milliseconds(1200));
    sp_loop->runLoop();

    EXPECT_FALSE(run);
}

TEST(RequestPool, NotTimeout_2)
{
    auto sp_loop = Loop::New();
    SetScopeExitAction([=] {delete sp_loop;});

    RequestPool<int> rp(sp_loop);
    rp.initialize(milliseconds(100), 10);

    auto token = rp.newRequest(new int(123));
    bool run = false;
    rp.setTimeoutAction([&] (int *p) { run = true; (void)p; });

    sp_loop->runInLoop([&] {
        auto req = rp.removeRequest(token);
        EXPECT_TRUE(req != nullptr);
        EXPECT_EQ(*req, 123);
        delete req;
    });

    sp_loop->exitLoop(milliseconds(1200));
    sp_loop->runLoop();

    EXPECT_FALSE(run);
}

//! 测试是否会有漏回调
TEST(RequestPool, Lost)
{
    auto sp_loop = Loop::New();
    SetScopeExitAction([=] {delete sp_loop;});

    RequestPool<int> rp(sp_loop);
    rp.initialize(milliseconds(100), 10);

    for (int i = 0; i < 100; ++i)
        rp.newRequest(new int(123));

    int count = 0;
    rp.setTimeoutAction([&] (int *p) { ++count; (void)p; });

    sp_loop->exitLoop(milliseconds(1200));
    sp_loop->runLoop();

    EXPECT_EQ(count, 100);
}

}
}
}
