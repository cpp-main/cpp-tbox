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

#include "function_action.h"

namespace tbox {
namespace flow {

TEST(NonDelayAction, True) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  FunctionAction action(*loop, [] { return true; });
  bool is_callback = false;
  action.setFinishCallback(
    [&is_callback, loop] (bool is_succ) {
      EXPECT_TRUE(is_succ);
      is_callback = true;
      loop->exitLoop();
    }
  );
  action.start();

  loop->runLoop();
  EXPECT_TRUE(is_callback);
}

TEST(NonDelayAction, False) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  FunctionAction action(*loop, [] { return false; });
  bool is_callback = false;
  action.setFinishCallback(
    [&is_callback, loop] (bool is_succ) {
      EXPECT_FALSE(is_succ);
      is_callback = true;
      loop->exitLoop();
    }
  );
  action.start();

  loop->runLoop();
  EXPECT_TRUE(is_callback);
}

}
}
