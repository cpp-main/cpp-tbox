#include <gtest/gtest.h>
#include "timeout_monitor.h"
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

    TimeoutMonitor tm(sp_loop);
    tm.initialize(milliseconds(100), 10);

    auto start_time = steady_clock::now();

    cabinet::Token pushed_token(100, 1);
    bool run = false;
    tm.setCallback([&] (const cabinet::Token &token) {
        EXPECT_EQ(token, pushed_token);

        auto d = steady_clock::now() - start_time;
        EXPECT_GT(d, milliseconds(900));
        EXPECT_LT(d, milliseconds(1100));
        run = true;
    });

    tm.add(pushed_token);
    sp_loop->exitLoop(milliseconds(1200));
    sp_loop->runLoop();

    EXPECT_TRUE(run);
}

}
}
}
