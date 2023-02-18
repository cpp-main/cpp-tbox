#include <gtest/gtest.h>
#include "timeout_monitor.hpp"
#include <tbox/base/scope_exit.hpp>

namespace tbox {
namespace eventx {
namespace {

using namespace event;
using namespace std::chrono;

TEST(TimeoutMonitor, Basic)
{
    auto sp_loop = Loop::New();
    SetScopeExitAction([=] {delete sp_loop;});

    TimeoutMonitor<int> tm(sp_loop);
    tm.initialize(milliseconds(100), 10);

    auto start_time = steady_clock::now();

    bool run = false;
    tm.setCallback([&] (int value) {
        EXPECT_EQ(value, 100);

        auto d = steady_clock::now() - start_time;
        EXPECT_GT(d, milliseconds(900));
        EXPECT_LT(d, milliseconds(1100));
        run = true;
    });

    tm.add(100);
    sp_loop->exitLoop(milliseconds(1200));
    sp_loop->runLoop();

    EXPECT_TRUE(run);
}

}
}
}
