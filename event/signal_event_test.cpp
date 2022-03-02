#include <gtest/gtest.h>
#include <signal.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <tbox/base/log_output.h>

#include "loop.h"
#include "signal_event.h"
#include "timer_event.h"

using namespace std;
using namespace tbox::event;

const int kAcceptableError = 10;

TEST(SignalEvent, Oneshot)
{
    LogOutput_Initialize("test");

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
                raise(SIGUSR1);
            }
        );
        sp_loop->exitLoop(std::chrono::milliseconds(100));
        sp_loop->runLoop();

        EXPECT_EQ(run_time, 1);

        delete signal_event;
        delete sp_loop;
    }
}

TEST(SignalEvent, PersistWithTimerEvent)
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
                    raise(SIGUSR1);
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
                raise(SIGUSR1);
                raise(SIGUSR2);
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

TEST(SignalEvent, MultiThread)
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
                raise(SIGUSR1);
                raise(SIGUSR2);
            }
        );

        bool t1_run = false;
        //! t1 线程sleep 200ms
        auto t1 = std::thread(
            [&] {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                t1_run = true;
            }
        );

        //! t2 线程等待一个信号
        bool exit_thread = false;
        std::mutex lock;
        std::condition_variable cond_var;
        bool t2_run = false;
        auto t2 = std::thread(
            [&] {
                std::unique_lock<std::mutex> lk(lock);
                cond_var.wait(lk, [&]{ return exit_thread; });
                t2_run = true;
            }
        );


        sp_loop->exitLoop(std::chrono::milliseconds(100));
        sp_loop->runLoop();

        t1.join();
        {
            std::unique_lock<std::mutex> lk(lock);
            exit_thread = true;
            cond_var.notify_one();
        }
        t2.join();

        EXPECT_TRUE(t1_run);
        EXPECT_TRUE(t2_run);
        EXPECT_EQ(int_run_time, 1);
        EXPECT_EQ(term_run_time, 1);

        delete int_signal_event;
        delete term_signal_event;
        delete sp_loop;
    }
}

//! 同一种事件被多个信号事件监听
TEST(SignalEvent, OneSignalMultiEvents)
{
    //!TODO
}

//! 同多种事件被多个信号事件监听
TEST(SignalEvent, MultiSignalMultiEvents)
{
    //!TODO
}

//! 多线程下多个Loop的事件监听同一个信号
TEST(SignalEvent, OneSignalMultiLoop)
{
    //!TODO
}
