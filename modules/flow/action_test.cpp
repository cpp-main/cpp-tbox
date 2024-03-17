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

TEST(Action, StartFinish) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  class TestAction : public Action {
    public:
      explicit TestAction(event::Loop &loop) : Action(loop, "Test") { }
      virtual bool isReady() const override { return true; }
      virtual void onStart() override { finish(true, 100); }
  };

  TestAction action(*loop);

  bool is_callback = false;
  action.setFinishCallback(
    [&, loop] (bool is_succ, const Action::Reason &r, const Action::Trace &t) {
      EXPECT_TRUE(is_succ);

      EXPECT_EQ(r.code, 100);
      ASSERT_EQ(t.size(), 1);
      EXPECT_EQ(t[0].type, "Test");
      is_callback = true;
    }
  );
  action.start();

  loop->exitLoop(std::chrono::milliseconds(10));
  loop->runLoop();

  EXPECT_TRUE(is_callback);
}

TEST(Action, StartBlock) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  class TestAction : public Action {
    public:
      explicit TestAction(event::Loop &loop) : Action(loop, "Test") { }
      virtual bool isReady() const override { return true; }
      virtual void onStart() override {
        Action::onStart();
        block(Action::Reason(100, "block_on_start"));
      }
  };

  bool is_block = false;

  TestAction action(*loop);
  action.setBlockCallback(
    [&](const Action::Reason &r, const Action::Trace &t) {
      EXPECT_EQ(r.code, 100);
      EXPECT_EQ(r.message, "block_on_start");
      ASSERT_EQ(t.size(), 1);
      EXPECT_EQ(t[0].type, "Test");
      is_block = true;
    }
  );
  action.start();

  loop->exitLoop(std::chrono::milliseconds(10));
  loop->runLoop();

  EXPECT_TRUE(is_block);
  EXPECT_EQ(action.state(), Action::State::kPause);

  action.stop();
  EXPECT_EQ(action.state(), Action::State::kStoped);
}

TEST(Action, Timeout) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  class TestAction : public Action {
    public:
      explicit TestAction(event::Loop &loop) : Action(loop, "Test") { }
      virtual bool isReady() const override { return true; }
  };

  TestAction action(*loop);
  action.setTimeout(std::chrono::milliseconds(50));

  bool is_callback = false;
  std::chrono::steady_clock::time_point ts_start;
  std::chrono::steady_clock::time_point ts_timeout;

  action.setFinishCallback(
    [&, loop] (bool is_succ, const Action::Reason &r, const Action::Trace &t) {
      EXPECT_FALSE(is_succ);
      EXPECT_EQ(r.code, ACTION_REASON_ACTION_TIMEOUT);
      EXPECT_EQ(r.message, "ActionTimeout");
      ASSERT_EQ(t.size(), 1);
      EXPECT_EQ(t[0].type, "Test");

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
