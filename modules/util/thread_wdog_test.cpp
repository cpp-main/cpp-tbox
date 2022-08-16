#include <thread>
#include <gtest/gtest.h>

#include "thread_wdog.h"

using namespace tbox::util;

TEST(ThreadWDog, thread_die)
{
    bool is_die = false;
    ThreadWDog::SetThreadDieCallback(
        [&] (pid_t, const std::string&)
        { is_die = true; }
    );

    ThreadWDog::Start();
    ThreadWDog::Register("main", 3);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    ThreadWDog::Unregister();
    ThreadWDog::Stop();

    EXPECT_TRUE(is_die);
}

TEST(ThreadWDog, thread_ok)
{
    bool is_die = false;
    ThreadWDog::SetThreadDieCallback(
        [&] (pid_t, const std::string&)
        { is_die = true; }
    );

    ThreadWDog::Start();
    ThreadWDog::Register("main", 4);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ThreadWDog::Unregister();
    ThreadWDog::Stop();

    EXPECT_FALSE(is_die);
}
