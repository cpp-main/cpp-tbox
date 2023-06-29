#include <gtest/gtest.h>
#include <thread>

#include "time_counter.h"

namespace tbox {
namespace util {

TEST(TimeCounter, SetTimeCounter)
{
    SetTimeCounter();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    SetTimeCounter();
    SetTimeCounter();
}

TEST(TimeCounter, SetNamedTimeCounter)
{
    SetNamedTimeCounter(a);
    SetNamedTimeCounter(b);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    StopNamedTimeCounter(a);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

TEST(TimeCounter, SetTimeCounterWithThreshold)
{
    {
        SetTimeCounterWithThreshold(10000000);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    {
        SetTimeCounterWithThreshold(1000000);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

TEST(TimeCounter, SetNamedTimeCounterWithThreshold)
{
    SetNamedTimeCounterWithThreshold(a, 10000000);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    StopNamedTimeCounterWithThreshold(a);

    SetNamedTimeCounterWithThreshold(b, 1000000);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    StopNamedTimeCounterWithThreshold(b);
}

TEST(TimeCounter, TimeCounter)
{
    TimeCounter tc;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    tc.print("after 10 msec");

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    tc.print("after 20 sec");
}

TEST(TimeCounter, CpuTimeCounter)
{
    CpuTimeCounter tc;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    tc.print("after 10 msec");

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    tc.print("after 20 sec");

    tc.start();
    tc.print("do nothing");
}

}
}
