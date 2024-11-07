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
#include <tbox/base/scope_exit.hpp>

#include "parallel_action.h"
#include "function_action.h"
#include "sleep_action.h"
#include "dummy_action.h"

namespace tbox {
namespace flow {

TEST(ParallelAction, TwoSleepAction) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    auto *para_action = new ParallelAction(*loop);
    SetScopeExitAction([para_action] { delete para_action; });

    auto *sleep_action_1 = new SleepAction(*loop, std::chrono::milliseconds(300));
    auto *sleep_action_2 = new SleepAction(*loop, std::chrono::milliseconds(200));

    EXPECT_NE(para_action->addChild(sleep_action_1), -1);
    EXPECT_NE(para_action->addChild(sleep_action_2), -1);
    EXPECT_TRUE(para_action->isReady());

    para_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason&, const Action::Trace&) {
            EXPECT_TRUE(is_succ);
            loop->exitLoop();
        }
    );

    auto start_time = std::chrono::steady_clock::now();
    para_action->start();
    loop->runLoop();

    auto d = std::chrono::steady_clock::now() - start_time;
    EXPECT_GT(d, std::chrono::milliseconds(290));
    EXPECT_LT(d, std::chrono::milliseconds(310));
}

TEST(ParallelAction, SleepFunctionAction) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    auto para_action = new ParallelAction(*loop);
    SetScopeExitAction([para_action] { delete para_action; });

    bool nodelay_action_succ = false;
    auto sleep_action = new SleepAction(*loop, std::chrono::milliseconds(50));
    auto function_action = new FunctionAction(*loop,
        [&] {
            nodelay_action_succ = true;
            return true;
        }
    );

    EXPECT_NE(para_action->addChild(sleep_action), -1);
    EXPECT_NE(para_action->addChild(function_action), -1);
    EXPECT_TRUE(para_action->isReady());

    para_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason&, const Action::Trace&) {
            EXPECT_TRUE(is_succ);
            loop->exitLoop();
        }
    );

    auto start_time = std::chrono::steady_clock::now();
    para_action->start();
    loop->runLoop();

    auto d = std::chrono::steady_clock::now() - start_time;
    EXPECT_GT(d, std::chrono::milliseconds(45));
    EXPECT_LT(d, std::chrono::milliseconds(55));

    EXPECT_TRUE(nodelay_action_succ);
}

TEST(ParallelAction, AllFinish) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    auto para_action = new ParallelAction(*loop, ParallelAction::Mode::kAllFinish);
    SetScopeExitAction([para_action] { delete para_action; });

    auto ta1 = new DummyAction(*loop);
    auto ta2 = new DummyAction(*loop);
    auto ta3 = new DummyAction(*loop);
    para_action->addChild(ta1);
    para_action->addChild(ta2);
    para_action->addChild(ta3);

    para_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason&, const Action::Trace&) {
            EXPECT_TRUE(is_succ);
            loop->exitLoop();
        }
    );

    para_action->start();
    loop->runNext([ta1] { ta1->emitFinish(true); });
    loop->runNext([ta2] { ta2->emitFinish(false); });
    loop->runNext([ta3] { ta3->emitFinish(false); });
    loop->runLoop();

    EXPECT_EQ(ta1->state(), Action::State::kFinished);
    EXPECT_EQ(ta2->state(), Action::State::kFinished);
    EXPECT_EQ(ta3->state(), Action::State::kFinished);
    EXPECT_EQ(ta1->result(), Action::Result::kSuccess);
    EXPECT_EQ(ta2->result(), Action::Result::kFail);
    EXPECT_EQ(ta2->result(), Action::Result::kFail);
}

TEST(ParallelAction, AnyFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    auto para_action = new ParallelAction(*loop, ParallelAction::Mode::kAnyFail);
    SetScopeExitAction([para_action] { delete para_action; });

    auto ta1 = new DummyAction(*loop);
    auto ta2 = new DummyAction(*loop);
    auto ta3 = new DummyAction(*loop);
    para_action->addChild(ta1);
    para_action->addChild(ta2);
    para_action->addChild(ta3);

    para_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason&, const Action::Trace&) {
            EXPECT_TRUE(is_succ);
            loop->exitLoop();
        }
    );

    para_action->start();
    loop->runNext([ta1] { ta1->emitFinish(true); });
    loop->runNext([ta2] { ta2->emitFinish(false); });
    loop->runLoop();

    EXPECT_EQ(ta1->state(), Action::State::kFinished);
    EXPECT_EQ(ta1->result(), Action::Result::kSuccess);
    EXPECT_EQ(ta2->state(), Action::State::kFinished);
    EXPECT_EQ(ta2->result(), Action::Result::kFail);
    EXPECT_EQ(ta3->state(), Action::State::kStoped);
}

TEST(ParallelAction, AnySucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    auto para_action = new ParallelAction(*loop, ParallelAction::Mode::kAnySucc);
    SetScopeExitAction([para_action] { delete para_action; });

    auto ta1 = new DummyAction(*loop);
    auto ta2 = new DummyAction(*loop);
    auto ta3 = new DummyAction(*loop);
    para_action->addChild(ta1);
    para_action->addChild(ta2);
    para_action->addChild(ta3);

    para_action->setFinishCallback(
        [loop](bool is_succ, const Action::Reason&, const Action::Trace&) {
            EXPECT_TRUE(is_succ);
            loop->exitLoop();
        }
    );

    para_action->start();
    loop->runNext([ta1] { ta1->emitFinish(false); });
    loop->runNext([ta2] { ta2->emitFinish(true); });
    loop->runLoop();

    EXPECT_EQ(ta1->state(), Action::State::kFinished);
    EXPECT_EQ(ta1->result(), Action::Result::kFail);
    EXPECT_EQ(ta2->state(), Action::State::kFinished);
    EXPECT_EQ(ta2->result(), Action::Result::kSuccess);
    EXPECT_EQ(ta3->state(), Action::State::kStoped);
}

TEST(ParallelAction, Block) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    auto para_action = new ParallelAction(*loop, ParallelAction::Mode::kAnySucc);
    SetScopeExitAction([para_action] { delete para_action; });

    auto ta1 = new DummyAction(*loop);
    auto ta2 = new DummyAction(*loop);
    auto ta3 = new DummyAction(*loop);
    para_action->addChild(ta1);
    para_action->addChild(ta2);
    para_action->addChild(ta3);

    para_action->setBlockCallback(
        [=] (const Action::Reason &r, const Action::Trace &t) {
            EXPECT_EQ(r.code, 111);
            ASSERT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].id, ta2->id());
            loop->exitLoop();
        }
    );

    para_action->start();
    loop->runNext([ta1] { ta1->emitFinish(false); });
    loop->runNext([ta2] { ta2->emitBlock(Action::Reason(111)); });
    loop->runLoop();

    EXPECT_EQ(ta1->state(), Action::State::kFinished);
    EXPECT_EQ(ta1->result(), Action::Result::kFail);
    EXPECT_EQ(ta2->state(), Action::State::kPause);
    EXPECT_EQ(ta3->state(), Action::State::kPause);
}

}
}
