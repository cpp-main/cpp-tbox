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
#include <tbox/eventx/timer_pool.h>
#include <tbox/base/scope_exit.hpp>

#include "repeat_action.h"
#include "function_action.h"
#include "succ_fail_action.h"
#include "dummy_action.h"

namespace tbox {
namespace flow {

TEST(RepeatAction, IsReady) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    RepeatAction action(*loop, 1);
    EXPECT_FALSE(action.isReady());

    action.setChild(new SuccAction(*loop));
    EXPECT_TRUE(action.isReady());
}

/**
 *  int loop_times = 0;
 *  for (int i = 0; i < 100; ++i) {
 *    ++loop_times;
 *  }
 */
TEST(RepeatAction, FunctionActionRepeat3NoBreak) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    RepeatAction repeat_action(*loop, 3, RepeatAction::Mode::kNoBreak);

    int loop_times = 0;
    auto function_action = new FunctionAction(*loop,
        [&] {
            ++loop_times;
            return true;
        }
    );
    bool is_finished = false;
    repeat_action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_TRUE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_REPEAT_NO_TIMES);
            EXPECT_EQ(r.message, "RepeatNoTimes");
            EXPECT_EQ(t.size(), 1);
            is_finished = true;
        }
    );
    EXPECT_TRUE(repeat_action.setChild(function_action));
    EXPECT_TRUE(repeat_action.isReady());

    repeat_action.start();
    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    EXPECT_TRUE(is_finished);
    EXPECT_EQ(loop_times, 3);
    EXPECT_EQ(repeat_action.state(), Action::State::kFinished);
}

/**
 *  int loop_times = 0;
 *  while (true) {
 *    ++loop_times;
 *  }
 */
TEST(RepeatAction, FunctionActionForeverNoBreak) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    RepeatAction repeat_action(*loop, 0, RepeatAction::Mode::kNoBreak);

    int loop_times = 0;
    auto function_action = new FunctionAction(*loop,
        [&] {
            ++loop_times;
            return true;
        }
    );
    bool is_finished = false;
    repeat_action.setFinishCallback(
        [&] (bool, const Action::Reason &, const Action::Trace &) {
            is_finished = true;
        }
    );
    EXPECT_TRUE(repeat_action.setChild(function_action));
    EXPECT_TRUE(repeat_action.isReady());

    repeat_action.start();
    loop->exitLoop(std::chrono::milliseconds(1000));
    loop->runLoop();

    repeat_action.stop();

    EXPECT_FALSE(is_finished);
    EXPECT_GT(loop_times, 1000);
    EXPECT_EQ(repeat_action.state(), Action::State::kStoped);
}

/**
 *  int loop_times = 0;
 *  for (int i = 0; i < 5; ++i) {
 *    ++loop_times;
 *    if (!(loop_times < 3))
 *      return true;
 *  }
 */
TEST(RepeatAction, FunctionActionRepeat5BreakFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    RepeatAction repeat_action(*loop, 5, RepeatAction::Mode::kBreakFail);

    int loop_times = 0;
    auto function_action = new FunctionAction(*loop,
        [&] {
            ++loop_times;
            return loop_times < 3;
        }
    );
    bool is_finished = false;
    EXPECT_TRUE(repeat_action.setChild(function_action));
    EXPECT_TRUE(repeat_action.isReady());
    repeat_action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_FALSE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FUNCTION_ACTION);
            ASSERT_EQ(t.size(), 2);
            ASSERT_EQ(t[0].type, "Function");
            ASSERT_EQ(t[1].type, "Repeat");
            is_finished = true;
        }
    );

    repeat_action.start();
    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    EXPECT_TRUE(is_finished);
    EXPECT_EQ(loop_times, 3);
    EXPECT_EQ(repeat_action.state(), Action::State::kFinished);
}

/**
 *  int loop_times = 0;
 *  for (int i = 0; i < 5; ++i) {
 *    ++loop_times;
 *    if (loop_times >= 3)
 *      return true;
 *  }
 */
TEST(RepeatAction, FunctionActionRepeat5BreakSucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    RepeatAction repeat_action(*loop, 5, RepeatAction::Mode::kBreakSucc);

    int loop_times = 0;
    auto function_action = new FunctionAction(*loop,
        [&] {
            ++loop_times;
            return loop_times >= 3;
        }
    );

    EXPECT_TRUE(repeat_action.setChild(function_action));
    EXPECT_TRUE(repeat_action.isReady());
    bool is_finished = false;
    repeat_action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_TRUE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FUNCTION_ACTION);
            ASSERT_EQ(t.size(), 2);
            ASSERT_EQ(t[0].type, "Function");
            ASSERT_EQ(t[1].type, "Repeat");
            is_finished = true;
        }
    );

    repeat_action.start();
    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    EXPECT_TRUE(is_finished);
    EXPECT_EQ(loop_times, 3);
    EXPECT_EQ(repeat_action.state(), Action::State::kFinished);
}

TEST(RepeatAction, FinishPause) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    eventx::TimerPool timer_pool(loop);

    RepeatAction repeat_action(*loop, 5);

    int loop_times = 0;
    bool do_resume = false;

    auto dummy_action = new DummyAction(*loop);
    dummy_action->setStartCallback([&] {
        ++loop_times;

        timer_pool.doAfter(std::chrono::milliseconds(1), [&] {
            //! 同时发生动作结束与动作暂停的事件
            dummy_action->emitFinish(true);
            repeat_action.pause();
        });
        timer_pool.doAfter(std::chrono::milliseconds(10), [&] {
            do_resume = true;
            repeat_action.resume();
        });
    });

    EXPECT_TRUE(repeat_action.setChild(dummy_action));
    EXPECT_TRUE(repeat_action.isReady());

    bool is_finished = false;
    repeat_action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_REPEAT_NO_TIMES);
            EXPECT_EQ(r.message, "RepeatNoTimes");
            is_finished = true;
        }
    );

    repeat_action.start();
    loop->exitLoop(std::chrono::milliseconds(60));
    loop->runLoop();

    EXPECT_TRUE(is_finished);
    EXPECT_EQ(loop_times, 5);
    EXPECT_TRUE(do_resume);
}

}
}
