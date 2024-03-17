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

#include "loop_if_action.h"
#include "function_action.h"
#include "sleep_action.h"
#include "sequence_action.h"
#include "succ_fail_action.h"

namespace tbox {
namespace flow {

/**
 *  int remain = 10;
 *  while (remain > 0) {
 *    --remain;
 *  }
 */
TEST(LoopIfAction, LoopRemainTimes) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    LoopIfAction loop_if_action(*loop);

    int remain = 10;
    auto cond_action = new FunctionAction(*loop, [&] { return remain > 0; });
    auto exec_action = new FunctionAction(*loop, [&] { --remain; return true; });

    bool is_finished = false;
    EXPECT_TRUE(loop_if_action.setChildAs(cond_action, "if"));
    EXPECT_TRUE(loop_if_action.setChildAs(exec_action, "exec"));
    EXPECT_TRUE(loop_if_action.isReady());
    loop_if_action.setFinishCallback([&] (bool, const Action::Reason&, const Action::Trace&) { is_finished = true; });
    loop_if_action.start();

    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    EXPECT_EQ(loop_if_action.state(), Action::State::kFinished);
    EXPECT_TRUE(is_finished);
    EXPECT_EQ(remain, 0);
}

/**
 *  int remain = 10;
 *  while (remain > 0) {
 *    --remain;
 *    delay_ms(10);
 *  }
 */
TEST(LoopIfAction, MultiAction) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    LoopIfAction loop_if_action(*loop);

    int remain = 10;
    auto cond_action = new FunctionAction(*loop, [&] { return remain > 0; });
    auto exec_action = new SequenceAction(*loop);
    exec_action->addChild(new FunctionAction(*loop, [&] { --remain; return true; }));
    exec_action->addChild(new SleepAction(*loop, std::chrono::milliseconds(10)));

    bool is_finished = false;
    EXPECT_TRUE(loop_if_action.setChildAs(cond_action, "if"));
    EXPECT_TRUE(loop_if_action.setChildAs(exec_action, "exec"));
    EXPECT_TRUE(loop_if_action.isReady());
    loop_if_action.setFinishCallback([&] (bool, const Action::Reason&, const Action::Trace&) { is_finished = true; });
    loop_if_action.start();

    loop->exitLoop(std::chrono::milliseconds(200));
    loop->runLoop();

    EXPECT_EQ(loop_if_action.state(), Action::State::kFinished);
    EXPECT_TRUE(is_finished);
    EXPECT_EQ(remain, 0);
}

TEST(LoopIfAction, IsReady) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    LoopIfAction action(*loop);
    EXPECT_FALSE(action.isReady());

    action.setChildAs(new SuccAction(*loop), "if");
    EXPECT_FALSE(action.isReady());

    action.setChildAs(new SuccAction(*loop), "exec");
    EXPECT_TRUE(action.isReady());
}

}
}
