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
#include <fcntl.h>
#include <gtest/gtest.h>
#include <time.h>

#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>
#include "timer_fd.h"

namespace tbox {
namespace eventx {

using namespace std;
using namespace tbox::event;

const int kAcceptableError = 1;

TEST(TimerFd, Oneshot)
{
    auto sp_loop = Loop::New("epoll");
    auto timer_event = new TimerFd(sp_loop, "10");
    SetScopeExitAction([=] { delete timer_event; delete sp_loop; });

    EXPECT_FALSE(timer_event->enable());
    EXPECT_TRUE(timer_event->initialize(chrono::milliseconds(10)));
    EXPECT_TRUE(timer_event->enable());

    int run_time = 0;
    timer_event->setCallback([&]() { ++run_time; });

    sp_loop->exitLoop(std::chrono::milliseconds(100));
    sp_loop->runLoop();
    timer_event->disable();

    EXPECT_EQ(run_time, 1);
    EXPECT_FALSE(timer_event->isEnabled());
}

TEST(TimerFd, Persist)
{
    auto sp_loop = Loop::New("epoll");
    auto timer_event = new TimerFd(sp_loop, "10");
    SetScopeExitAction([=] { delete timer_event; delete sp_loop; });

    EXPECT_TRUE(timer_event->initialize(chrono::milliseconds(10), chrono::milliseconds(10)));
    EXPECT_TRUE(timer_event->enable());

    int run_time = 0;
    timer_event->setCallback([&run_time]() { ++run_time; });

    sp_loop->exitLoop(std::chrono::milliseconds(100));
    sp_loop->runLoop();

    EXPECT_EQ(run_time, 10);
    EXPECT_TRUE(timer_event->isEnabled());
}

TEST(TimerFd, DisableSelfInCallback)
{
    auto sp_loop = Loop::New("epoll");
    auto timer_event = new TimerFd(sp_loop, "10");
    SetScopeExitAction([=] { delete timer_event; delete sp_loop; });

    EXPECT_TRUE(timer_event->initialize(chrono::milliseconds(10), chrono::milliseconds(10)));
    EXPECT_TRUE(timer_event->enable());

    int run_time = 0;
    timer_event->setCallback(
        [&] () {
            timer_event->disable();
            ++run_time;
        }
    );

    sp_loop->exitLoop(std::chrono::milliseconds(100));
    sp_loop->runLoop();

    EXPECT_EQ(run_time, 1);
}

TEST(TimerFd, Precision)
{
    auto sp_loop = Loop::New("epoll");
    auto timer_event = new TimerFd(sp_loop, "100");
    SetScopeExitAction([=] { delete timer_event; delete sp_loop; });

    EXPECT_TRUE(timer_event->initialize(chrono::milliseconds(100), chrono::milliseconds(100)));
    EXPECT_TRUE(timer_event->enable());

    int count = 0;
    auto start_time = chrono::steady_clock::now();
    timer_event->setCallback(
        [&] {
            ++count;

            auto d = chrono::steady_clock::now() - start_time;
            EXPECT_GT(d, chrono::milliseconds(count * 100 - kAcceptableError));
            EXPECT_LT(d, chrono::milliseconds(count * 100 + kAcceptableError));

            if (count >= 20)
                sp_loop->exitLoop();
        }
    );

    sp_loop->runLoop();
}

TEST(TimerFd, NanoSeconds)
{
    struct timespec ts;

    // Get number of nanoseconds from last second to the present
    ASSERT_EQ(clock_gettime(CLOCK_MONOTONIC, &ts), 0) << "Failed to get clock time";

    auto ns = ts.tv_nsec;
    auto prev_ns = ns - (ns % 1000000);
    auto min_interval_ns = ns - prev_ns;
    printf("Elapsed nanoseconds since last second: %ld\n", min_interval_ns);

    auto sp_loop = Loop::New("epoll");
    auto timer_event = new TimerFd(sp_loop, std::to_string(min_interval_ns));
    SetScopeExitAction([=] { delete timer_event; delete sp_loop; });

    EXPECT_TRUE(timer_event->initialize(chrono::nanoseconds(min_interval_ns)));

    std::chrono::steady_clock::time_point start_ts, stop_ts;

    EXPECT_TRUE(timer_event->enable());
    start_ts = std::chrono::steady_clock::now();

    timer_event->setCallback(
        [&] {
            stop_ts = std::chrono::steady_clock::now();
            timer_event->disable();
            sp_loop->exitLoop();
        }
    );

    sp_loop->runLoop();

    uint64_t elapsed_ns = (stop_ts - start_ts).count();
    ASSERT_GE(elapsed_ns, min_interval_ns) << "Timer did not expire after " << min_interval_ns << "nanoseconds" << endl;
    ASSERT_LE(elapsed_ns, 2 * min_interval_ns) << "Timer expired too late, elapsed_ns=" << elapsed_ns << endl;
}

}
}
