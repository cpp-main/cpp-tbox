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
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/eventx/timer_pool.h>
#include <tbox/base/scope_exit.hpp>

#include "sequence_action.h"
#include "function_action.h"
#include "sleep_action.h"
#include "dummy_action.h"

namespace tbox {
namespace flow {

TEST(SequenceAction, FinishIfAnyFail_AllSucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    bool action_run_1 = false;
    bool action_run_2 = false;

    auto *seq_action = new SequenceAction(*loop);
    SetScopeExitAction([seq_action] { delete seq_action; });

    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            action_run_1 = true;
            return true;
        }
    ));
    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            EXPECT_TRUE(action_run_1);
            action_run_2 = true;
            return true;
        }
    ));
    EXPECT_TRUE(seq_action->isReady());
    seq_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_TRUE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FUNCTION_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Function");
            EXPECT_EQ(t[1].type, "Sequence");
            loop->exitLoop();
        }
    );
    seq_action->setMode(SequenceAction::Mode::kAnyFail);
    seq_action->start();

    loop->runLoop();
    EXPECT_TRUE(action_run_2);
    EXPECT_EQ(seq_action->index(), 2);
}

TEST(SequenceAction, FinishIfAnyFail_FailHead) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    bool action_run_1 = false;
    bool action_run_2 = false;

    auto *seq_action = new SequenceAction(*loop);
    SetScopeExitAction([seq_action] { delete seq_action; });

    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            action_run_1 = true;
            return false;
        }
    ));
    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            action_run_2 = true;
            return false;
        }
    ));
    EXPECT_TRUE(seq_action->isReady());
    seq_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_FALSE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FUNCTION_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Function");
            EXPECT_EQ(t[1].type, "Sequence");
            loop->exitLoop();
        }
    );
    seq_action->setMode(SequenceAction::Mode::kAnyFail);
    seq_action->start();

    loop->runLoop();
    EXPECT_TRUE(action_run_1);
    EXPECT_FALSE(action_run_2);
    EXPECT_EQ(seq_action->index(), 0);
}


TEST(SequenceAction, FinishIfAnyFail_FailTail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    bool action_run_1 = false;
    bool action_run_2 = false;

    auto *seq_action = new SequenceAction(*loop);
    SetScopeExitAction([seq_action] { delete seq_action; });

    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            action_run_1 = true;
            return true;
        }
    ));
    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            EXPECT_TRUE(action_run_1);
            action_run_2 = true;
            return false;
        }
    ));
    EXPECT_TRUE(seq_action->isReady());
    seq_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_FALSE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FUNCTION_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Function");
            EXPECT_EQ(t[1].type, "Sequence");
            loop->exitLoop();
        }
    );
    seq_action->setMode(SequenceAction::Mode::kAnyFail);
    seq_action->start();

    loop->runLoop();
    EXPECT_TRUE(action_run_2);
    EXPECT_EQ(seq_action->index(), 1);
}

TEST(SequenceAction, TwoSleepAction) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    auto *seq_action = new SequenceAction(*loop);
    SetScopeExitAction([seq_action] { delete seq_action; });

    auto *sleep_action_1 = new SleepAction(*loop, std::chrono::milliseconds(300));
    auto *sleep_action_2 = new SleepAction(*loop, std::chrono::milliseconds(200));

    seq_action->addChild(sleep_action_1);
    seq_action->addChild(sleep_action_2);
    EXPECT_TRUE(seq_action->isReady());

    seq_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_TRUE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_SLEEP_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Sleep");
            EXPECT_EQ(t[1].type, "Sequence");
            loop->exitLoop();
        }
    );

    auto start_time = std::chrono::steady_clock::now();
    seq_action->start();
    loop->runLoop();

    auto d = std::chrono::steady_clock::now() - start_time;
    EXPECT_GT(d, std::chrono::milliseconds(490));
    EXPECT_LT(d, std::chrono::milliseconds(510));

    EXPECT_EQ(seq_action->index(), 2);
}

TEST(SequenceAction, FinishIfAnySucc_AllFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    bool action_run_1 = false;
    bool action_run_2 = false;

    auto *seq_action = new SequenceAction(*loop);
    SetScopeExitAction([seq_action] { delete seq_action; });

    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            action_run_1 = true;
            return false;
        }
    ));
    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            EXPECT_TRUE(action_run_1);
            action_run_2 = true;
            return false;
        }
    ));
    EXPECT_TRUE(seq_action->isReady());
    seq_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_FALSE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FUNCTION_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Function");
            EXPECT_EQ(t[1].type, "Sequence");
            loop->exitLoop();
        }
    );
    seq_action->setMode(SequenceAction::Mode::kAnySucc);
    seq_action->start();

    loop->runLoop();
    EXPECT_TRUE(action_run_2);
    EXPECT_EQ(seq_action->index(), 2);
}

TEST(SequenceAction, FinishIfAnySucc_SuccHead) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    bool action_run_1 = false;
    bool action_run_2 = false;

    auto *seq_action = new SequenceAction(*loop);
    SetScopeExitAction([seq_action] { delete seq_action; });

    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            action_run_1 = true;
            return true;
        }
    ));
    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            action_run_2 = true;
            return false;
        }
    ));
    EXPECT_TRUE(seq_action->isReady());
    seq_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_TRUE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FUNCTION_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Function");
            EXPECT_EQ(t[1].type, "Sequence");
            loop->exitLoop();
        }
    );
    seq_action->setMode(SequenceAction::Mode::kAnySucc);
    seq_action->start();

    loop->runLoop();
    EXPECT_TRUE(action_run_1);
    EXPECT_FALSE(action_run_2);
    EXPECT_EQ(seq_action->index(), 0);
}

TEST(SequenceAction, FinishIfAnySucc_SuccTail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    bool action_run_1 = false;
    bool action_run_2 = false;

    auto *seq_action = new SequenceAction(*loop);
    SetScopeExitAction([seq_action] { delete seq_action; });

    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            action_run_1 = true;
            return false;
        }
    ));
    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            EXPECT_TRUE(action_run_1);
            action_run_2 = true;
            return true;
        }
    ));
    EXPECT_TRUE(seq_action->isReady());
    seq_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_TRUE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FUNCTION_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Function");
            EXPECT_EQ(t[1].type, "Sequence");
            loop->exitLoop();
        }
    );
    seq_action->setMode(SequenceAction::Mode::kAnySucc);
    seq_action->start();

    loop->runLoop();
    EXPECT_TRUE(action_run_2);
    EXPECT_EQ(seq_action->index(), 1);
}

TEST(SequenceAction, FinishIfAllFinish_AllFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    bool action_run_1 = false;
    bool action_run_2 = false;

    auto *seq_action = new SequenceAction(*loop);
    SetScopeExitAction([seq_action] { delete seq_action; });

    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            action_run_1 = true;
            return false;
        }
    ));
    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            EXPECT_TRUE(action_run_1);
            action_run_2 = true;
            return false;
        }
    ));
    EXPECT_TRUE(seq_action->isReady());
    seq_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_FALSE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FUNCTION_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Function");
            EXPECT_EQ(t[1].type, "Sequence");
            loop->exitLoop();
        }
    );
    seq_action->setMode(SequenceAction::Mode::kAllFinish);
    seq_action->start();

    loop->runLoop();
    EXPECT_TRUE(action_run_2);
    EXPECT_EQ(seq_action->index(), 2);
}

TEST(SequenceAction, FinishIfAllFinish_AllSucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    bool action_run_1 = false;
    bool action_run_2 = false;

    auto *seq_action = new SequenceAction(*loop);
    SetScopeExitAction([seq_action] { delete seq_action; });

    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            action_run_1 = true;
            return true;
        }
    ));
    seq_action->addChild(new FunctionAction(*loop,
        [&] {
            EXPECT_TRUE(action_run_1);
            action_run_2 = true;
            return true;
        }
    ));
    EXPECT_TRUE(seq_action->isReady());
    seq_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_TRUE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FUNCTION_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Function");
            EXPECT_EQ(t[1].type, "Sequence");
            loop->exitLoop();
        }
    );
    seq_action->setMode(SequenceAction::Mode::kAllFinish);
    seq_action->start();

    loop->runLoop();
    EXPECT_TRUE(action_run_2);
    EXPECT_EQ(seq_action->index(), 2);
}

//! 两个子动作，第一个子动作出现了finish与pause
//! 观察第二个子动作有没有正常执行
TEST(SequenceAction, FinishPause) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    eventx::TimerPool timer_pool(loop);

    SequenceAction seq_action(*loop, SequenceAction::Mode::kAnyFail);

    bool is_dummy_action_run = false;
    bool is_function_action_run = false;
    bool do_resume = false;

    auto dummy_action = new DummyAction(*loop);
    dummy_action->setStartCallback([&] {
        is_dummy_action_run = true;
        timer_pool.doAfter(std::chrono::milliseconds(1), [&] {
            //! 同时发生动作结束与动作暂停的事件
            dummy_action->emitFinish(true);
            seq_action.pause();
        });
        timer_pool.doAfter(std::chrono::milliseconds(10), [&] {
            do_resume = true;
            seq_action.resume();
        });
    });

    EXPECT_EQ(seq_action.addChild(dummy_action), 0);
    EXPECT_EQ(seq_action.addChild(new FunctionAction(*loop, [&] { is_function_action_run = true; return true; })), 1);
    EXPECT_TRUE(seq_action.isReady());

    bool is_finished = false;
    seq_action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            is_finished = true;
        }
    );

    seq_action.start();
    loop->exitLoop(std::chrono::milliseconds(15));
    loop->runLoop();

    EXPECT_TRUE(is_dummy_action_run);
    EXPECT_TRUE(is_function_action_run);
    EXPECT_TRUE(is_finished);
    EXPECT_TRUE(do_resume);
}

}
}
