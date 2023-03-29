#include <gtest/gtest.h>
#include <thread>

#include "time_counter.h"
#include <tbox/base/log_output.h>

TEST(TimeCounter, SetTimeCounter)
{
    LogOutput_Initialize();

    SetTimeCounter();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    SetTimeCounter();
    
    LogOutput_Cleanup();
}

TEST(TimeCounter, SetNamedTimeCounter)
{
    LogOutput_Initialize();

    SetNamedTimeCounter(a);
    SetNamedTimeCounter(b);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    StopNamedTimeCounter(a);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    LogOutput_Cleanup();
}

TEST(TimeCounter, SetTimeCounterWithThreshold)
{
    LogOutput_Initialize();

    {
        SetTimeCounterWithThreshold(std::chrono::milliseconds(10));
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    {
        SetTimeCounterWithThreshold(std::chrono::milliseconds(10));
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    LogOutput_Cleanup();
}

TEST(TimeCounter, SetNamedTimeCounterWithThreshold)
{
    LogOutput_Initialize();

    SetNamedTimeCounterWithThreshold(a, std::chrono::milliseconds(10));
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    StopNamedTimeCounterWithThreshold(a);

    SetNamedTimeCounterWithThreshold(b, std::chrono::milliseconds(10));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    StopNamedTimeCounterWithThreshold(b);

    LogOutput_Cleanup();
}
