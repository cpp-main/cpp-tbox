#include <gtest/gtest.h>
#include <signal.h>

#include "loop.h"
#include "signal_event.h"
#include "timer_event.h"

using namespace std;
using namespace tbox::event;

const int kAcceptableError = 10;

TEST(SignalEvent, Oneshot)
{
    auto engins = Loop::Engines();
    for (auto e : engins) {
        cout << "engin: " << e << endl;
        auto sp_loop = Loop::New(e);
        auto signal_event = sp_loop->newSignalEvent();
        EXPECT_TRUE(signal_event->initialize(SIGUSR1, Event::Mode::kOneshot));
        EXPECT_TRUE(signal_event->enable());

        int run_time = 0;
        signal_event->setCallback([&]() { ++run_time; });

        sp_loop->run([]
            {
                pid_t pid = getpid();
                kill(pid, SIGUSR1);
            }
        );
        sp_loop->exitLoop(std::chrono::milliseconds(100));
        sp_loop->runLoop();

        EXPECT_EQ(run_time, 1);

        delete signal_event;
        delete sp_loop;
    }
}

TEST(SignalEvent, Persist)
{
    auto engins = Loop::Engines();
    for (auto e : engins) {
        cout << "engin: " << e << endl;
        auto sp_loop = Loop::New(e);
        auto signal_event = sp_loop->newSignalEvent();
        EXPECT_TRUE(signal_event->initialize(SIGUSR1, Event::Mode::kPersist));
        EXPECT_TRUE(signal_event->enable());

        auto timer_event = sp_loop->newTimerEvent();
        EXPECT_TRUE(timer_event->initialize(chrono::milliseconds(10), Event::Mode::kPersist));
        EXPECT_TRUE(timer_event->enable());
        int count = 0;
        timer_event->setCallback([&]
            {
                ++count;
                if (count <= 5) {
                    pid_t pid = getpid();
                    kill(pid, SIGUSR1);
                }
            }
        );

        int run_time = 0;
        signal_event->setCallback([&]() { ++run_time; });

        sp_loop->exitLoop(std::chrono::milliseconds(100));
        sp_loop->runLoop();

        EXPECT_EQ(run_time, 5);

        delete timer_event;
        delete signal_event;
        delete sp_loop;
    }
}

TEST(SignalEvent, IntAndTermSignal)
{
    auto engins = Loop::Engines();
    for (auto e : engins) {
        cout << "engin: " << e << endl;
        auto sp_loop = Loop::New(e);
        auto int_signal_event = sp_loop->newSignalEvent();
        EXPECT_TRUE(int_signal_event->initialize(SIGUSR1, Event::Mode::kOneshot));
        EXPECT_TRUE(int_signal_event->enable());

        auto term_signal_event = sp_loop->newSignalEvent();
        EXPECT_TRUE(term_signal_event->initialize(SIGUSR2, Event::Mode::kOneshot));
        EXPECT_TRUE(term_signal_event->enable());

        int int_run_time = 0;
        int_signal_event->setCallback([&]() { ++int_run_time; });
        int term_run_time = 0;
        term_signal_event->setCallback([&]() { ++term_run_time; });

        sp_loop->run([]
            {
                pid_t pid = getpid();
                kill(pid, SIGUSR1);
                kill(pid, SIGUSR2);
            }
        );
        sp_loop->exitLoop(std::chrono::milliseconds(100));
        sp_loop->runLoop();

        EXPECT_EQ(int_run_time, 1);
        EXPECT_EQ(term_run_time, 1);

        delete int_signal_event;
        delete term_signal_event;
        delete sp_loop;
    }
}

