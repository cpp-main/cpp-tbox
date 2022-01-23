#include <gtest/gtest.h>
#include <thread>

#include "loop.h"
#include "timer_event.h"

#include <tbox/base/log.h>
#include <tbox/base/scope_exit.hpp>

using namespace std;
using namespace std::chrono;

using namespace tbox;
using namespace tbox::event;

TEST(CommonLoop, isRunning)
{
    Loop *sp_loop = event::Loop::New();
    TimerEvent *sp_timer = sp_loop->newTimerEvent();
    SetScopeExitAction([sp_loop, sp_timer]{ delete sp_loop; delete sp_timer; });

    sp_timer->initialize(chrono::milliseconds(10), Event::Mode::kOneshot);
    bool is_run = false;
    sp_timer->setCallback(
        [sp_loop, &is_run] {
            is_run = true;
            EXPECT_TRUE(sp_loop->isRunning());
        }
    );
    sp_timer->enable();

    EXPECT_FALSE(sp_loop->isRunning());

    sp_loop->exitLoop(chrono::milliseconds(50));
    sp_loop->runLoop();

    EXPECT_TRUE(is_run);
    EXPECT_FALSE(sp_loop->isRunning());
}

TEST(CommonLoop, isInLoopThread)
{
    Loop *sp_loop = event::Loop::New();
    TimerEvent *sp_timer = sp_loop->newTimerEvent();
    SetScopeExitAction([sp_loop, sp_timer]{ delete sp_loop; delete sp_timer; });

    sp_timer->initialize(chrono::milliseconds(10), Event::Mode::kOneshot);
    bool is_timer_run = false;
    sp_timer->setCallback(
        [sp_loop, &is_timer_run] {
            is_timer_run = true;
            EXPECT_TRUE(sp_loop->isInLoopThread());
        }
    );
    sp_timer->enable();

    bool is_thread_run = false;
    auto t = thread(
        [sp_loop, &is_thread_run] {
            is_thread_run = true;
            this_thread::sleep_for(chrono::milliseconds(10));
            EXPECT_FALSE(sp_loop->isInLoopThread());
        }
    );

    sp_loop->exitLoop(chrono::milliseconds(50));
    sp_loop->runLoop();

    EXPECT_TRUE(is_timer_run);
    EXPECT_TRUE(is_thread_run);

    t.join();
}

TEST(CommonLoop, runNext)
{
    LogUndo();
}

TEST(CommonLoop, runInLoop)
{
    LogUndo();
}

TEST(CommonLoop, cleanupUndoTasks)
{
    LogUndo();
}

