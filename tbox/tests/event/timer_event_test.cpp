#include <gtest/gtest.h>
#include <unistd.h>
#include <fcntl.h>

#include "loop.h"
#include "timer_event.h"

using namespace std;
using namespace tbox::event;

const int kAcceptableError = 10;

TEST(TimerEvent, Oneshot)
{
    auto engins = Loop::Engines();
    for (auto e : engins) {
        cout << "engin: " << e << endl;
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
    auto engins = Loop::Engines();
    for (auto e : engins) {
        cout << "engin: " << e << endl;
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
    auto engins = Loop::Engines();
    for (auto e : engins) {
        cout << "engin: " << e << endl;
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
    auto engins = Loop::Engines();
    for (auto e : engins) {
        cout << "engin: " << e << endl;
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
