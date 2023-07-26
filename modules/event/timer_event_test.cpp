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
#include <unistd.h>
#include <fcntl.h>

#include "loop.h"
#include "timer_event.h"

namespace tbox {
namespace event {

using namespace std;

const int kAcceptableError = 10;

TEST(TimerEvent, Oneshot)
{
    auto engines = Loop::Engines();
    for (auto e : engines) {
        cout << "engine: " << e << endl;
        auto sp_loop = Loop::New(e);
        auto timer_event = sp_loop->newTimerEvent();
        EXPECT_TRUE(timer_event->initialize(chrono::milliseconds(10), Event::Mode::kOneshot));
        EXPECT_TRUE(timer_event->enable());

        int run_time = 0;
        timer_event->setCallback([&]() { ++run_time; });

        sp_loop->exitLoop(std::chrono::milliseconds(100));
        sp_loop->runLoop();

        EXPECT_EQ(run_time, 1);
        EXPECT_FALSE(timer_event->isEnabled());

        delete timer_event;
        delete sp_loop;
    }
}

TEST(TimerEvent, Persist)
{
    auto engines = Loop::Engines();
    for (auto e : engines) {
        cout << "engine: " << e << endl;
        auto sp_loop = Loop::New(e);
        auto timer_event = sp_loop->newTimerEvent();
        EXPECT_TRUE(timer_event->initialize(chrono::milliseconds(10), Event::Mode::kPersist));
        EXPECT_TRUE(timer_event->enable());

        int run_time = 0;
        timer_event->setCallback([&]() { ++run_time; });

        sp_loop->exitLoop(std::chrono::milliseconds(100));
        sp_loop->runLoop();

        EXPECT_EQ(run_time, 10);
        EXPECT_TRUE(timer_event->isEnabled());

        delete timer_event;
        delete sp_loop;
    }
}

TEST(TimerEvent, DisableSelfInCallback)
{
    auto engines = Loop::Engines();
    for (auto e : engines) {
        cout << "engine: " << e << endl;
        auto sp_loop = Loop::New(e);
        auto timer_event = sp_loop->newTimerEvent();
        EXPECT_TRUE(timer_event->initialize(chrono::milliseconds(10), Event::Mode::kPersist));
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

        EXPECT_EQ(run_time, 1); //! 应该只执行一次

        delete timer_event;
        delete sp_loop;
    }
}

TEST(TimerEvent, Precision)
{
    auto engines = Loop::Engines();
    for (auto e : engines) {
        cout << "engine: " << e << endl;
        auto sp_loop = Loop::New(e);
        auto timer_event = sp_loop->newTimerEvent();
        EXPECT_TRUE(timer_event->initialize(chrono::milliseconds(100), Event::Mode::kPersist));
        EXPECT_TRUE(timer_event->enable());

        int count = 0;
        auto start_time = chrono::steady_clock::now();
        timer_event->setCallback(
            [&] {
                ++count;

                auto d = chrono::steady_clock::now() - start_time;
                EXPECT_GT(d, chrono::milliseconds(count * 100 - kAcceptableError));
                EXPECT_LT(d, chrono::milliseconds(count * 100 + kAcceptableError));

                if (count == 20)
                    sp_loop->exitLoop();
            }
        );
        sp_loop->runLoop();

        delete timer_event;
        delete sp_loop;
    }
}

}
}
