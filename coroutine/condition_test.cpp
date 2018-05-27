#include <gtest/gtest.h>
#include "condition.h"
#include <tbox/base/scope_exit.hpp>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::coroutine;

//! 模拟生产者、消费者
TEST(Condition, OneWakeAnother)
{
    Loop *sp_loop = event::Loop::New();
    SetScopeExitAction([sp_loop]{ delete sp_loop;});

    Scheduler sch(sp_loop);

    int times = 20;
    Condition cond(sch);
    //! 生产者
    auto routine1_entry = [&] (Scheduler &sch) {
        for (int i = 0; i < times; ++i) {
            cond = true;    //! 激活 cond 绑定的协程
            sch.yield();
        }
    };

    //! 消费者
    int count = 0;
    auto routine2_entry = [&] (Scheduler &sch) {
        while (!sch.isCanceled()) {
            if (cond) {     //! 检查是不是 cond 触发的
                ++count;
                cond = false;   //! 将 cond 复位
            } else {
                sch.wait();
            }
        }
    };

    sch.create(routine1_entry, true, "r1");
    auto r2 = sch.create(routine2_entry, true, "r2");

    cond.bind(r2);  //! cond 绑定 r2

    sp_loop->exitLoop(chrono::milliseconds(100));
    sp_loop->runLoop();

    EXPECT_EQ(count, times);
} 
