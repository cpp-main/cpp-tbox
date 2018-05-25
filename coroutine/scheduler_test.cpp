#include <gtest/gtest.h>
#include "scheduler.h"
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/base/scope_exit.hpp>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::coroutine;

TEST(Scheduler, CreateTwoRoutineThenStop)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    int exec_count = 0;
    {
        Scheduler sch(sp_loop);
        auto entry = [&exec_count] (Scheduler &sch) {
            ++exec_count;
        };
        sch.create(entry, "test1");
        sch.create(entry, "test2");

        sp_loop->exitLoop(chrono::milliseconds(10));
        sp_loop->runLoop();
    }

    EXPECT_EQ(exec_count, 0);
}

TEST(Scheduler, CreateTwoRoutineStartOneThenStop)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    int exec_count = 0;
    {
        Scheduler sch(sp_loop);
        auto entry = [&exec_count] (Scheduler &sch) {
            ++exec_count;
        };
        auto key1 = sch.create(entry, "test1");
        sch.resume(key1);
        sch.create(entry, "test2");

        sp_loop->exitLoop(chrono::milliseconds(10));
        sp_loop->runLoop();
    }

    EXPECT_EQ(exec_count, 1);
}

//! 测试在协程中是否能获取name与key
TEST(Scheduler, GetInfoInRoutine)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    string read_name;
    RoutineKey  read_key;

    auto entry = [&] (Scheduler &sch) {
        read_name = sch.getName();
        read_key = sch.getKey();
    };

    auto key1 = sch.create(entry, "test1");
    sch.resume(key1);
    sch.create(entry, "test2");

    sp_loop->exitLoop(chrono::milliseconds(20));
    sp_loop->runLoop();

    EXPECT_EQ(read_name, "test1");
    EXPECT_TRUE(read_key.equal(key1));
}

//! 测试在协程中创建另一个协程，观察被创建的协程是否被执行
TEST(Scheduler, RoutineCreateAnotherRoutine)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    bool routine1_run = false;
    auto routine1_entry = [&] (Scheduler &sch) {
        routine1_run = true;
    };

    bool routine2_end = false;
    auto routine2_entry = [&] (Scheduler &sch) {
        auto key = sch.create(routine1_entry, "be created routine");
        sch.resume(key);
        sch.yield();
        routine2_end = true;
    };


    auto key = sch.create(routine2_entry);
    sch.resume(key);

    sp_loop->exitLoop(chrono::milliseconds(20));
    sp_loop->runLoop();

    EXPECT_TRUE(routine1_run);
    EXPECT_TRUE(routine2_end);
}

//! 测试 Scheduler的yield()
TEST(Scheduler, Yield)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    int routine1_count = 0;
    auto routine1_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < 20; ++i) {
            ++routine1_count;
            sch.yield();
        }
    };
    int routine2_count = 0;
    auto routine2_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < 10; ++i) {
            ++routine2_count;
            sch.yield();
        }
    };

    sch.resume(sch.create(routine1_entry));
    sch.resume(sch.create(routine2_entry));

    sp_loop->exitLoop(chrono::seconds(1));
    sp_loop->runLoop();

    EXPECT_EQ(routine1_count, 20);
    EXPECT_EQ(routine2_count, 10);
}

//! 测试 Scheduler::wait() 功能
TEST(Scheduler, Wait)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    bool routine_begin = false;
    bool routine_end = false;
    auto routine_entry = [&] (Scheduler &sch) {
        routine_begin = true;
        sch.wait();
        routine_end = true;
    };
    auto key = sch.create(routine_entry);
    sch.resume(key);

    //! 创建定时器，1秒后唤醒协程
    auto timer = sp_loop->newTimerEvent();
    SetScopeExitAction([timer]{ delete timer;});
    timer->initialize(chrono::milliseconds(10), Event::Mode::kOneshot);
    timer->setCallback(
        [&] {
            EXPECT_TRUE(routine_begin);
            EXPECT_FALSE(routine_end);
            sch.resume(key);
            sp_loop->exitLoop(chrono::milliseconds(10));    //! 不能直接停，要预留一点时间
        }
    );
    timer->enable();

    sp_loop->runLoop();

    EXPECT_TRUE(routine_begin);
    EXPECT_TRUE(routine_end);
}

//! 测试 Scheduler的yield()与cancel()功能
TEST(Scheduler, CancelRoutineByTimer)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    bool routine_stop = false;
    int  count = 0;
    auto routine_entry = [&] (Scheduler &sch) {
        while (true) {
            ++count;
            sch.yield();
            if (sch.isCanceled())
                break;
        }
        routine_stop = true;
    };
    auto key = sch.create(routine_entry);
    sch.resume(key);

    //! 创建定时器，1秒后取消协程
    auto timer = sp_loop->newTimerEvent();
    SetScopeExitAction([timer]{ delete timer;});
    timer->initialize(chrono::milliseconds(10), Event::Mode::kOneshot);
    timer->setCallback(
        [&] {
            sch.cancel(key);
            sp_loop->exitLoop(chrono::milliseconds(10));    //! 不能直接停，要预留一点时间
        }
    );
    timer->enable();

    sp_loop->runLoop();

    EXPECT_NE(count, 0);
    EXPECT_TRUE(routine_stop);
}
