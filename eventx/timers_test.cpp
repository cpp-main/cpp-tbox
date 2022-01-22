#include <gtest/gtest.h>
#include "timers.h"
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/base/scope_exit.hpp>

using namespace std;
using namespace std::chrono;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::eventx;

/**
 * 创建一个100ms的周期性定时任务
 * 在每次执行的时候，检查任务是否在 N * 100 ms 左右
 * 并且执行了10次。
 */
TEST(Timers, doEvery)
{
    Loop *sp_loop = event::Loop::New();
    Timers timers(sp_loop);
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    auto start_time = system_clock::now();
    int count = 0;
    Timers::Token token;
    token = timers.doEvery(milliseconds(100),
        [&] (const Timers::Token &t){
            EXPECT_EQ(t, token);
            auto d = system_clock::now() - start_time;
            EXPECT_GT(d, milliseconds(count * 100 + 90));
            EXPECT_LT(d, milliseconds(count * 100 + 110));
            ++count;
        }
    );
    sp_loop->exitLoop(chrono::milliseconds(1010));
    sp_loop->runLoop();

    timers.cleanup();

    EXPECT_EQ(count, 10);
}

/**
 * 先创建一个 500ms 的单次执行任务
 * 在任务执行中，检查执行的时间范围是否在 490~510ms 之间
 */
TEST(Timers, doAfter)
{
    Loop *sp_loop = event::Loop::New();
    Timers timers(sp_loop);
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    auto start_time = system_clock::now();
    Timers::Token token;
    bool is_run = false;
    token = timers.doAfter(milliseconds(500),
        [&] (const Timers::Token &t){
            EXPECT_EQ(t, token);
            auto d = system_clock::now() - start_time;
            EXPECT_GT(d, milliseconds(490));
            EXPECT_LT(d, milliseconds(510));
            is_run = true;
        }
    );
    sp_loop->exitLoop(chrono::milliseconds(1500));
    sp_loop->runLoop();

    timers.cleanup();
    EXPECT_TRUE(is_run);
}

/**
 * 先创建一个 100ms 的定时任务。
 * 再创建一个 50ms 的定时器，在任务中取消上一个定时任务。
 * 观察 is_run 是否为 false
 */
TEST(Timers, cancel_inside_loop)
{
    Loop *sp_loop = event::Loop::New();
    Timers timers(sp_loop);
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    bool is_run = false;
    auto token = timers.doAfter(milliseconds(100),
        [&] (const Timers::Token &){
            is_run = true;
        }
    );

    auto t = sp_loop->newTimerEvent();
    SetScopeExitAction([t]{ delete t;});
    t->initialize(chrono::milliseconds(50), Event::Mode::kOneshot);
    t->setCallback([&] { timers.cancel(token); });
    t->enable();

    sp_loop->exitLoop(chrono::milliseconds(200));
    sp_loop->runLoop();

    timers.cleanup();

    EXPECT_FALSE(is_run);
}

/**
 * 先创建一个 100ms 的定时任务。
 * 在执行 runLoop() 之前就 timers.cancel()
 * 观察 is_run 是否为 false
 */
TEST(Timers, cancel_outside_loop)
{
    Loop *sp_loop = event::Loop::New();
    Timers timers(sp_loop);
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    bool is_run = false;
    auto token = timers.doAfter(milliseconds(100),
        [&] (const Timers::Token &){
            is_run = true;
        }
    );

    timers.cancel(token);
    sp_loop->exitLoop(chrono::milliseconds(200));
    sp_loop->runLoop();

    timers.cleanup();

    EXPECT_FALSE(is_run);
}

/**
 * 创建一个1000ms后的任务。在该任务执行的时候检查执行时间是否在 990 ~ 1010 ms 之间
 * 在 1500ms 后停止
 */
TEST(Timers, doAt)
{
    Loop *sp_loop = event::Loop::New();
    Timers timers(sp_loop);
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    auto start_time = system_clock::now();
    Timers::Token token;
    bool is_run = false;
    token = timers.doAt(start_time + milliseconds(1000),
        [&] (const Timers::Token &t){
            EXPECT_EQ(t, token);
            auto d = system_clock::now() - start_time;
            EXPECT_GT(d, milliseconds(990));
            EXPECT_LT(d, milliseconds(1010));
            is_run = true;
        }
    );
    sp_loop->exitLoop(chrono::milliseconds(1500));
    sp_loop->runLoop();

    timers.cleanup();

    EXPECT_TRUE(is_run);
}

/**
 * 创建一个周期性100ms的任务，每次任务执行的时候都检查执行的时间点是否在 N * 100ms 左右。
 * 再创建第二个单次510ms的任务，去取消第一个周期性任务。
 * 最后创建第三个单次1010ms的任务，停止 loop。
 */
TEST(Timers, all)
{
    Loop *sp_loop = event::Loop::New();
    Timers timers(sp_loop);
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    auto start_time = system_clock::now();

    int count = 0;
    Timers::Token token = timers.doEvery(milliseconds(100),
        [&] (const Timers::Token &t) {
            auto d = system_clock::now() - start_time;
            EXPECT_GT(d, milliseconds(count * 100 + 90));
            EXPECT_LT(d, milliseconds(count * 100 + 110));
            ++count;
        }
    );

    bool is_run = false;
    timers.doAfter(milliseconds(510),
        [&] (const Timers::Token &t) {
            auto d = system_clock::now() - start_time;
            EXPECT_GT(d, milliseconds(500));
            EXPECT_LT(d, milliseconds(520));
            timers.cancel(token);
            is_run = true;
        }
    );

    timers.doAfter(milliseconds(1010),
        [&] (const Timers::Token &) {
            sp_loop->exitLoop();
        }
    );

    sp_loop->runLoop();

    auto d = system_clock::now() - start_time;
    EXPECT_GT(d, milliseconds(1000));
    EXPECT_LT(d, milliseconds(1020));

    timers.cleanup();

    EXPECT_EQ(count, 5);
    EXPECT_TRUE(is_run);
}
