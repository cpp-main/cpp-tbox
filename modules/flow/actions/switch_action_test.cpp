/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/eventx/timer_pool.h>

#include "switch_action.h"
#include "function_action.h"
#include "succ_fail_action.h"
#include "dummy_action.h"

namespace tbox {
namespace flow {

TEST(SwitchAction, IsReady) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    SwitchAction action(*loop);
    EXPECT_FALSE(action.isReady());

    action.setChildAs(new SuccAction(*loop), "switch");
    EXPECT_FALSE(action.isReady());

    action.setChildAs(new SuccAction(*loop), "default");
    EXPECT_TRUE(action.isReady());

    action.setChildAs(new SuccAction(*loop), "case:a");
    EXPECT_TRUE(action.isReady());
}

TEST(SwitchAction, SwitchFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    SwitchAction action(*loop);

    bool switch_action_run = false;
    bool case_action_run = false;
    bool default_action_run = false;

    auto switch_action = new FunctionAction(*loop, [&] { switch_action_run = true; return false; });
    auto case_action = new FunctionAction(*loop, [&] { case_action_run = true; return true; });
    auto default_action = new FunctionAction(*loop, [&] { default_action_run = true; return true; });

    EXPECT_TRUE(action.setChildAs(switch_action, "switch"));
    EXPECT_TRUE(action.setChildAs(case_action, "case:A"));
    EXPECT_TRUE(action.setChildAs(default_action, "default"));

    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &) {
            EXPECT_FALSE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_SWITCH_FAIL);
            EXPECT_EQ(r.message, "SwitchFail");
            switch_action_run = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();

    EXPECT_TRUE(switch_action_run);
    EXPECT_FALSE(case_action_run);
    EXPECT_FALSE(default_action_run);
}

TEST(SwitchAction, SwitchCase) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    SwitchAction action(*loop);

    bool switch_action_run = false;
    bool case_a_action_run = false;
    bool case_b_action_run = false;
    bool default_action_run = false;

    //! switch动作在退出之前设置了r.message为case:B
    auto switch_action = new FunctionAction(*loop,
        [&] (Action::Reason &r) {
            switch_action_run = true;
            r.message = "case:B";
            return true;
        }
    );
    EXPECT_TRUE(action.setChildAs(switch_action, "switch"));

    auto case_a_action = new FunctionAction(*loop, [&] { case_a_action_run = true; return true; });
    auto case_b_action = new FunctionAction(*loop, [&] { case_b_action_run = true; return true; });
    auto default_action = new FunctionAction(*loop, [&] { default_action_run = true; return true; });

    EXPECT_TRUE(action.setChildAs(case_a_action, "case:A"));
    EXPECT_TRUE(action.setChildAs(case_b_action, "case:B"));
    EXPECT_TRUE(action.setChildAs(default_action, "default"));

    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            switch_action_run = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();

    EXPECT_TRUE(switch_action_run);
    EXPECT_FALSE(case_a_action_run);
    EXPECT_TRUE(case_b_action_run);
    EXPECT_FALSE(default_action_run);
}

TEST(SwitchAction, SwitchDefault) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    SwitchAction action(*loop);

    bool switch_action_run = false;
    bool case_action_run = false;
    bool default_action_run = false;

    //! switch动作返回时没有设置r.message
    auto switch_action = new FunctionAction(*loop, [&] { switch_action_run = true; return true; });
    auto case_action = new FunctionAction(*loop, [&] { case_action_run = true; return true; });
    auto default_action = new FunctionAction(*loop, [&] { default_action_run = true; return true; });

    EXPECT_TRUE(action.setChildAs(switch_action, "switch"));
    EXPECT_TRUE(action.setChildAs(case_action, "case:A"));
    EXPECT_TRUE(action.setChildAs(default_action, "default"));

    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            switch_action_run = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();

    EXPECT_TRUE(switch_action_run);
    EXPECT_FALSE(case_action_run);
    EXPECT_TRUE(default_action_run);
}

TEST(SwitchAction, SwitchSkip) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    SwitchAction action(*loop);

    bool switch_action_run = false;
    bool case_action_run = false;

    //! switch动作返回时没有设置r.message
    auto switch_action = new FunctionAction(*loop, [&] { switch_action_run = true; return true; });
    auto case_action = new FunctionAction(*loop, [&] { case_action_run = true; return true; });

    EXPECT_TRUE(action.setChildAs(switch_action, "switch"));
    EXPECT_TRUE(action.setChildAs(case_action, "case:A"));

    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &) {
            EXPECT_FALSE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_SWITCH_SKIP);
            EXPECT_EQ(r.message, "SwitchSkip");
            switch_action_run = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();

    EXPECT_TRUE(switch_action_run);
    EXPECT_FALSE(case_action_run);
}

/**
 * 在switch动作开始后，pause一次，再resume；
 * 在case动作开始后，pause一次，再resume；
 */
TEST(SwitchAction, PauseResume) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    SwitchAction action(*loop);

    bool switch_action_start = false;
    bool switch_action_pause = false;
    bool switch_action_resume = false;
    bool case_action_start = false;
    bool case_action_pause = false;
    bool case_action_resume = false;
    bool default_action_run = false;
    bool all_done = false;

    auto switch_action = new DummyAction(*loop);
    auto case_action = new DummyAction(*loop);
    auto default_action = new FunctionAction(*loop, [&] { default_action_run = true; return true; });

    EXPECT_TRUE(action.setChildAs(switch_action, "switch"));
    EXPECT_TRUE(action.setChildAs(case_action, "case:A"));
    EXPECT_TRUE(action.setChildAs(default_action, "default"));

    //! 当switch被启动时，则暂停
    switch_action->setStartCallback([&] {
        switch_action_start = true;
        loop->runNext([&] { action.pause(); });
    });
    //! 当switch被暂停时，则恢复
    switch_action->setPauseCallback([&] {
        switch_action_pause = true;
        loop->runNext([&] { action.resume();});
    });
    //! 当switch被恢复时，则完成，并选择case:A
    switch_action->setResumeCallback([&] {
        switch_action_resume = true;
        loop->runNext([&] {
            Action::Reason r;
            r.message = "case:A";
            switch_action->emitFinish(true, r);
        });
    });

    //! 当case被启动时，则暂停
    case_action->setStartCallback([&] {
        case_action_start = true;
        loop->runNext([&] { action.pause(); });
    });
    //! 当case被暂停时，则恢复
    case_action->setPauseCallback([&] {
        case_action_pause = true;
        loop->runNext([&] { action.resume(); });
    });
    //! 当case被恢复时，则完成
    case_action->setResumeCallback([&] {
        case_action_resume = true;
        loop->runNext([&] { case_action->emitFinish(true); });
    });

    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            all_done = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();

    EXPECT_TRUE(switch_action_start);
    EXPECT_TRUE(switch_action_pause);
    EXPECT_TRUE(switch_action_resume);
    EXPECT_TRUE(case_action_start);
    EXPECT_TRUE(case_action_pause);
    EXPECT_TRUE(case_action_resume);
    EXPECT_FALSE(default_action_run);
    EXPECT_TRUE(all_done);
}

//! 在执行switch动作的时候，被stop了
//! 观察后继的动作还会不会继续做
TEST(SwitchAction, StopOnSwitch) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    SwitchAction action(*loop);

    bool switch_action_start = false;
    bool switch_action_stop = false;
    bool case_action_run = false;
    bool all_done = false;

    auto switch_action = new DummyAction(*loop);
    auto case_action = new FunctionAction(*loop, [&] { case_action_run = true; return true; });

    EXPECT_TRUE(action.setChildAs(switch_action, "switch"));
    EXPECT_TRUE(action.setChildAs(case_action, "case:A"));

    //! 当switch被启动时，则停止
    switch_action->setStartCallback([&] {
        switch_action_start = true;
        loop->runNext([&] { action.stop(); });
    });
    //! 当switch被停止时，则中断Loop
    switch_action->setStopCallback([&] {
        switch_action_stop = true;
        loop->exitLoop(std::chrono::milliseconds(1));
    });

    action.setFinishCallback(
        [&] (bool, const Action::Reason &, const Action::Trace &) {
            all_done = true;
        }
    );

    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();

    EXPECT_TRUE(switch_action_start);
    EXPECT_TRUE(switch_action_stop);

    EXPECT_FALSE(case_action_run);
    EXPECT_FALSE(all_done);
}

//! 在执行default动作的时候，被stop了
//! 观察后继的动作还会不会继续做
TEST(SwitchAction, StopOnDefault) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    SwitchAction action(*loop);

    bool switch_action_run = false;
    bool default_action_start = false;
    bool default_action_stop = false;
    bool all_done = false;

    auto switch_action = new FunctionAction(*loop, [&] { switch_action_run = true; return true; });
    auto default_action = new DummyAction(*loop);

    EXPECT_TRUE(action.setChildAs(switch_action, "switch"));
    EXPECT_TRUE(action.setChildAs(default_action, "default"));

    //! 当default被启动时，则停止
    default_action->setStartCallback([&] {
        default_action_start = true;
        loop->runNext([&] { action.stop(); });
    });
    //! 当default被停止时，则中断Loop
    default_action->setStopCallback([&] {
        default_action_stop = true;
        loop->exitLoop(std::chrono::milliseconds(1));
    });

    action.setFinishCallback(
        [&] (bool, const Action::Reason &, const Action::Trace &) {
            all_done = true;
        }
    );

    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();

    EXPECT_TRUE(switch_action_run);
    EXPECT_TRUE(default_action_start);
    EXPECT_TRUE(default_action_stop);
    EXPECT_FALSE(all_done);
}

/**
 * 在switch动作开始后，pause一次，再resume；
 * 在case动作开始后，pause一次，再resume；
 */
TEST(SwitchAction, FinishPauseOnSwitch) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    eventx::TimerPool timer_pool(loop);

    SwitchAction action(*loop);

    bool switch_action_start = false;
    bool default_action_run = false;
    bool all_done = false;
    bool do_resume = false;

    auto switch_action = new DummyAction(*loop);
    auto default_action = new FunctionAction(*loop, [&] { default_action_run = true; return true; });

    EXPECT_TRUE(action.setChildAs(switch_action, "switch"));
    EXPECT_TRUE(action.setChildAs(default_action, "default"));

    //! 当switch被启动后1ms，同时触发finish与pause
    //! 之后1ms，又恢复action
    switch_action->setStartCallback([&] {
        switch_action_start = true;
        timer_pool.doAfter(std::chrono::milliseconds(1), [&] {
            //! 同时发生动作结束与动作暂停的事件
            switch_action->emitFinish(true, Action::Reason("case:A"));
            action.pause();
        });
        timer_pool.doAfter(std::chrono::milliseconds(10), [&] {
            do_resume = true;
            action.resume();
        });
    });

    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            all_done = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();

    EXPECT_TRUE(switch_action_start);
    EXPECT_TRUE(default_action_run);
    EXPECT_TRUE(all_done);
    EXPECT_TRUE(do_resume);
}

}
}
