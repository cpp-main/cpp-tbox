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
#include <iostream>

#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

#include "composite_action.h"
#include "sleep_action.h"
#include "loop_action.h"
#include "function_action.h"
#include "sequence_action.h"
#include "succ_fail_action.h"

namespace tbox {
namespace flow {

TEST(CompositeAction, Basic) {
    class TimeCountTestAction : public CompositeAction {
      public:
        TimeCountTestAction(event::Loop &loop) : CompositeAction(loop, "TimeCount") {
            auto loop_action = new LoopAction(loop);
            auto seq_action = new SequenceAction(loop);
            seq_action->addChild(new SleepAction(loop, std::chrono::milliseconds(100)));
            seq_action->addChild(new FunctionAction(loop, [this] { ++count_; return true; }));
            loop_action->setChild(seq_action);
            setChild(loop_action);
        }

        virtual void onReset() {
            count_ = 0;
            CompositeAction::onReset();
        }

        int count() const { return count_; }

      private:
        int count_ = 0;
    };

    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    TimeCountTestAction action(*loop);
    EXPECT_TRUE(action.isReady());

    action.start();

    loop->exitLoop(std::chrono::milliseconds(1010));
    loop->runLoop();

    action.stop();
    EXPECT_GE(action.count(), 10);
    loop->cleanup();
}

//! 测试父动作提前结束动作的情况，观察有没有stop子动作
TEST(CompositeAction, ParentFinishBeforeChild) {

    class TimeoutTestAction : public CompositeAction {
      public:
        TimeoutTestAction(event::Loop &loop, Action *child)
          : CompositeAction(loop, "Timeout") {
          setChild(child);
          setTimeout(std::chrono::milliseconds(10));
        }
    };

    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    auto sleep_action = new SleepAction(*loop, std::chrono::seconds(1));
    TimeoutTestAction action(*loop, sleep_action);
    action.start();

    loop->exitLoop(std::chrono::milliseconds(100));
    loop->runLoop();

    EXPECT_EQ(action.state(), Action::State::kFinished);
    EXPECT_EQ(sleep_action->state(), Action::State::kStoped);
}

TEST(CompositeAction, ChildBlock) {
    //! 该子动作，第一次启动的时候，会block，后面恢复的时候会finish
    class CanBlockAction : public Action {
      public:
        explicit CanBlockAction(event::Loop &loop) : Action(loop, "CanBlock") { }

        virtual bool isReady() const { return true; }
        virtual void onStart() override { block(1); }
        virtual void onResume() override { finish(true); }
    };

    class ParentAction : public CompositeAction {
      public:
        explicit ParentAction(event::Loop &loop)
          : CompositeAction(loop, "Parent") {
          setChild(new CanBlockAction(loop));
        }
    };

    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    bool is_finished = false;
    bool is_blocked = false;

    ParentAction action(*loop);

    action.setBlockCallback(
        [&] (const Action::Reason &why, const Action::Trace &) {
            is_blocked = true;
            EXPECT_EQ(why.code, 1);
            EXPECT_FALSE(is_finished);
            EXPECT_EQ(action.state(), Action::State::kPause);
            action.resume();
        }
    );
    action.setFinishCallback(
        [&] (bool succ, const Action::Reason &, const Action::Trace &) {
            is_finished = true;
            EXPECT_TRUE(succ);
            EXPECT_TRUE(is_blocked);
            EXPECT_EQ(action.state(), Action::State::kFinished);
        }
    );

    action.start();

    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    EXPECT_TRUE(is_blocked);
    EXPECT_TRUE(is_finished);
    EXPECT_EQ(action.state(), Action::State::kFinished);
}

TEST(CompositeAction, NotSetChild) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    CompositeAction action(*loop, "UnSet");
    EXPECT_FALSE(action.isReady());
}

TEST(CompositeAction, ReasonAndTrace) {
    class TestAction : public CompositeAction {
      public:
        explicit TestAction(event::Loop &loop)
          : CompositeAction(loop, "Test") {
          setChild(new FailAction(loop));
        }
    };

    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    TestAction action(*loop);
    action.set_label("hello");

    bool is_finished = false;
    action.setFinishCallback(
        [&](bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_FALSE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FAIL_ACTION);
            EXPECT_EQ(r.message, "FailAction");

            ASSERT_EQ(t.size(), 2);
            EXPECT_EQ(t[0].type, "Fail");
            EXPECT_EQ(t[1].type, "Test");
            std::cout << ToString(r) << " -- " << ToString(t) << std::endl;

            is_finished = true;
        }
    );

    action.start();

    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    EXPECT_TRUE(is_finished);
}

}
}
