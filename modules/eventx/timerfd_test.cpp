#include <unistd.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <time.h>

#include <tbox/event/loop.h>
#include "timerfd.h"

namespace tbox {
namespace eventx {

using namespace std;
using namespace tbox::event;

const int kAcceptableError = 10;

TEST(TimerFd, Oneshot)
{
    auto sp_loop = Loop::New("epoll");
    auto timer_event = new TimerFd(sp_loop, "10");
    EXPECT_FALSE(timer_event->enable());
    EXPECT_TRUE(timer_event->initialize(chrono::milliseconds(10), Event::Mode::kOneshot));
    EXPECT_TRUE(timer_event->enable());

    int run_time = 0;
    timer_event->setCallback([&]() { ++run_time; });

    sp_loop->exitLoop(std::chrono::milliseconds(100));
    sp_loop->runLoop();
    timer_event->disable();

    EXPECT_EQ(run_time, 1);
    EXPECT_FALSE(timer_event->isEnabled());

    delete timer_event;
    delete sp_loop;
}

TEST(TimerFd, Persist)
{
    auto sp_loop = Loop::New("epoll");
    auto timer_event = new TimerFd(sp_loop, "10");
    EXPECT_TRUE(timer_event->initialize(chrono::milliseconds(10), Event::Mode::kPersist));
    EXPECT_TRUE(timer_event->enable());

    int run_time = 0;
    timer_event->setCallback([&run_time]() { ++run_time; });

    sp_loop->exitLoop(std::chrono::milliseconds(100));
    sp_loop->runLoop();

    EXPECT_EQ(run_time, 10);
    EXPECT_TRUE(timer_event->isEnabled());

    delete timer_event;
    delete sp_loop;
}

TEST(TimerFd, DisableSelfInCallback)
{
    auto sp_loop = Loop::New("epoll");
    auto timer_event = new TimerFd(sp_loop, "10");
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

    EXPECT_EQ(run_time, 1);

    delete timer_event;
    delete sp_loop;
}

TEST(TimerFd, Precision)
{
    auto sp_loop = Loop::New("epoll");
    auto timer_event = new TimerFd(sp_loop, "100");
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

            if (count >= 20)
            sp_loop->exitLoop();
        }
    );

    sp_loop->runLoop();
    delete timer_event;
    delete sp_loop;
}

TEST(TimerFd, NanoSeconds)
{
    struct timespec ts;

    // Get number of nanoseconds from last second to the present
    ASSERT_EQ(clock_gettime(CLOCK_MONOTONIC, &ts), 0) << "Failed to get clock time";

    long ns = ts.tv_nsec;
    long prev_ns = ns - (ns % 1000000);
    long min_interval_ns = ns - prev_ns;
    printf("Elapsed nanoseconds since last second: %ld\n", min_interval_ns);
    auto sp_loop = Loop::New("epoll");
    auto timer_event = new TimerFd(sp_loop, std::to_string(min_interval_ns));
    EXPECT_TRUE(timer_event->initialize(chrono::nanoseconds(min_interval_ns), Event::Mode::kOneshot));
    timer_event->setCallback(
        [&] {
            timer_event->disable();
            sp_loop->exitLoop();
        }
    );

    EXPECT_TRUE(timer_event->enable());

    struct timespec before, after;
    ASSERT_EQ(clock_gettime(CLOCK_MONOTONIC, &before), 0) << "Failed to get clock time";
    sp_loop->runLoop();
    ASSERT_EQ(clock_gettime(CLOCK_MONOTONIC, &after), 0) << "Failed to get clock time";
    uint64_t elapsed_ns = (after.tv_sec - before.tv_sec) * (uint64_t)1000000000 + (after.tv_nsec - before.tv_nsec);
    ASSERT_GE(elapsed_ns, min_interval_ns) << "Timer did not expire after " << min_interval_ns << "nanoseconds" << endl;
    ASSERT_LE(elapsed_ns, 2 * min_interval_ns) << "Timer expired too late, elapsed_ns=" << elapsed_ns << endl;
    delete timer_event;
    delete sp_loop;
}

}
}
