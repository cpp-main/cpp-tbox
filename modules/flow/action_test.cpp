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
#include "action.h"

namespace tbox {
namespace flow {

TEST(Action, Timeout) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  class TestAction : public Action {
    public:
      explicit TestAction(event::Loop &loop) : Action(loop, "Test") { }
  };

  TestAction action(*loop);
  action.setTimeout(std::chrono::milliseconds(50));

  bool is_callback = false;
  std::chrono::steady_clock::time_point ts_start;
  std::chrono::steady_clock::time_point ts_timeout;

  action.setFinishCallback(
    [&, loop] (bool is_succ) {
      EXPECT_FALSE(is_succ);
      is_callback = true;
      ts_timeout = std::chrono::steady_clock::now();
      loop->exitLoop();
    }
  );
  action.start();
  ts_start = std::chrono::steady_clock::now();

  loop->runLoop();
  EXPECT_TRUE(is_callback);

  auto cout_50 = std::chrono::duration_cast<std::chrono::milliseconds>(ts_timeout - ts_start).count();
  EXPECT_LE(cout_50, 51);
  EXPECT_GE(cout_50, 49);
}

}
}
