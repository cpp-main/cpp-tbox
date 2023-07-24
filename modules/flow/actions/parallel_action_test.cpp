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

namespace tbox {
namespace flow {

TEST(ParallelAction, TwoSleepAction) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  auto *para_action = new ParallelAction(*loop);
  SetScopeExitAction([para_action] { delete para_action; });

  auto *sleep_action_1 = new SleepAction(*loop, std::chrono::milliseconds(300));
  auto *sleep_action_2 = new SleepAction(*loop, std::chrono::milliseconds(200));

  para_action->append(sleep_action_1);
  para_action->append(sleep_action_2);

  para_action->setFinishCallback(
    [loop](bool is_succ) {
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

  auto *para_action = new ParallelAction(*loop);
  SetScopeExitAction([para_action] { delete para_action; });

  bool nodelay_action_succ = false;
  auto *sleep_action = new SleepAction(*loop, std::chrono::milliseconds(50));
  auto *function_action = new FunctionAction(*loop,
    [&] {
      nodelay_action_succ = true;
      return true;
    }
  );

  para_action->append(sleep_action);
  para_action->append(function_action);

  para_action->setFinishCallback(
    [loop](bool is_succ) {
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

}
}
