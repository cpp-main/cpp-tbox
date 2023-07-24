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

  int remain = 10;
  auto cond_action = new FunctionAction(*loop, [&] { return remain > 0; });
  auto exec_action = new FunctionAction(*loop, [&] { --remain; return true; });
  LoopIfAction loop_if_action(*loop, cond_action, exec_action);

  bool is_finished = false;
  loop_if_action.setFinishCallback([&] (bool) { is_finished = true; });
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

  int remain = 10;
  auto cond_action = new FunctionAction(*loop, [&] { return remain > 0; });
  auto exec_action = new SequenceAction(*loop);
  exec_action->append(new FunctionAction(*loop, [&] { --remain; return true; }));
  exec_action->append(new SleepAction(*loop, std::chrono::milliseconds(10)));
  LoopIfAction loop_if_action(*loop, cond_action, exec_action);

  bool is_finished = false;
  loop_if_action.setFinishCallback([&] (bool) { is_finished = true; });
  loop_if_action.start();

  loop->exitLoop(std::chrono::milliseconds(200));
  loop->runLoop();

  EXPECT_EQ(loop_if_action.state(), Action::State::kFinished);
  EXPECT_TRUE(is_finished);
  EXPECT_EQ(remain, 0);
}


}
}
