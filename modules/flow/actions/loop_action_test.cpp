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

#include "loop_action.h"
#include "function_action.h"
#include "sleep_action.h"
#include "sequence_action.h"

namespace tbox {
namespace flow {

/**
 *  int loop_times = 0;
 *  while (true) {
 *    ++loop_times;
 *    return true;
 *  }
 */
TEST(LoopAction, FunctionActionForever) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  int loop_times = 0;
  auto function_action = new FunctionAction(*loop,
    [&] {
      ++loop_times;
      return true;
    }
  );
  LoopAction loop_action(*loop, function_action, LoopAction::Mode::kForever);
  bool is_finished = false;
  loop_action.setFinishCallback([&] (bool) { is_finished = true; });

  loop_action.start();
  loop->exitLoop(std::chrono::milliseconds(1000));
  loop->runLoop();
  loop_action.stop();


  EXPECT_FALSE(is_finished);
  EXPECT_GT(loop_times, 1000);
}

/**
 *  int loop_times = 0;
 *  while (true) {
 *    ++loop_times;
 *    Sleep(100);
 *  };
 */
TEST(LoopAction, SleepActionForever) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  int loop_times = 0;
  auto function_action = new FunctionAction(*loop,
    [&] {
      ++loop_times;
      return true;
    }
  );
  auto delay_10ms_action = new SleepAction(*loop, std::chrono::milliseconds(100));
  auto seq_action = new SequenceAction(*loop);
  seq_action->append(delay_10ms_action);
  seq_action->append(function_action);

  LoopAction loop_action(*loop, seq_action, LoopAction::Mode::kForever);
  bool is_finished = false;
  loop_action.setFinishCallback([&] (bool) { is_finished = true; });

  loop_action.start();
  loop->exitLoop(std::chrono::milliseconds(1010));
  loop->runLoop();
  loop_action.stop();

  EXPECT_FALSE(is_finished);
  EXPECT_EQ(loop_times, 10);
}

}
}
