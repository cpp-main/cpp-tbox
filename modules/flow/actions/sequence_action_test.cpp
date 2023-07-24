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

#include "sequence_action.h"
#include "function_action.h"
#include "sleep_action.h"

namespace tbox {
namespace flow {

TEST(SequenceAction, FinishIfAnyFail_AllSucc) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool action_run_1 = false;
  bool action_run_2 = false;

  auto *seq_action = new SequenceAction(*loop);
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new FunctionAction(*loop,
    [&] {
      action_run_1 = true;
      return true;
    }
  ));
  seq_action->append(new FunctionAction(*loop,
    [&] {
      EXPECT_TRUE(action_run_1);
      action_run_2 = true;
      return true;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_succ) {
      EXPECT_TRUE(is_succ);
      loop->exitLoop();
    }
  );
  seq_action->setFinishCondition(SequenceAction::FinishCondition::kAnyFail);
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_run_2);
  EXPECT_EQ(seq_action->index(), 2);
}

TEST(SequenceAction, FinishIfAnyFail_FailHead) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool action_run_1 = false;
  bool action_run_2 = false;

  auto *seq_action = new SequenceAction(*loop);
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new FunctionAction(*loop,
    [&] {
      action_run_1 = true;
      return false;
    }
  ));
  seq_action->append(new FunctionAction(*loop,
    [&] {
      action_run_2 = true;
      return false;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_succ) {
      EXPECT_FALSE(is_succ);
      loop->exitLoop();
    }
  );
  seq_action->setFinishCondition(SequenceAction::FinishCondition::kAnyFail);
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_run_1);
  EXPECT_FALSE(action_run_2);
  EXPECT_EQ(seq_action->index(), 0);
}


TEST(SequenceAction, FinishIfAnyFail_FailTail) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool action_run_1 = false;
  bool action_run_2 = false;

  auto *seq_action = new SequenceAction(*loop);
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new FunctionAction(*loop,
    [&] {
      action_run_1 = true;
      return true;
    }
  ));
  seq_action->append(new FunctionAction(*loop,
    [&] {
      EXPECT_TRUE(action_run_1);
      action_run_2 = true;
      return false;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_succ) {
      EXPECT_FALSE(is_succ);
      loop->exitLoop();
    }
  );
  seq_action->setFinishCondition(SequenceAction::FinishCondition::kAnyFail);
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_run_2);
  EXPECT_EQ(seq_action->index(), 1);
}

TEST(SequenceAction, TwoSleepAction) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  auto *seq_action = new SequenceAction(*loop);
  SetScopeExitAction([seq_action] { delete seq_action; });

  auto *sleep_action_1 = new SleepAction(*loop, std::chrono::milliseconds(300));
  auto *sleep_action_2 = new SleepAction(*loop, std::chrono::milliseconds(200));

  seq_action->append(sleep_action_1);
  seq_action->append(sleep_action_2);

  seq_action->setFinishCallback(
    [loop](bool is_succ) {
      EXPECT_TRUE(is_succ);
      loop->exitLoop();
    }
  );

  auto start_time = std::chrono::steady_clock::now();
  seq_action->start();
  loop->runLoop();

  auto d = std::chrono::steady_clock::now() - start_time;
  EXPECT_GT(d, std::chrono::milliseconds(490));
  EXPECT_LT(d, std::chrono::milliseconds(510));

  EXPECT_EQ(seq_action->index(), 2);
}

TEST(SequenceAction, FinishIfAnySucc_AllFail) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool action_run_1 = false;
  bool action_run_2 = false;

  auto *seq_action = new SequenceAction(*loop);
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new FunctionAction(*loop,
    [&] {
      action_run_1 = true;
      return false;
    }
  ));
  seq_action->append(new FunctionAction(*loop,
    [&] {
      EXPECT_TRUE(action_run_1);
      action_run_2 = true;
      return false;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_succ) {
      EXPECT_FALSE(is_succ);
      loop->exitLoop();
    }
  );
  seq_action->setFinishCondition(SequenceAction::FinishCondition::kAnySucc);
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_run_2);
  EXPECT_EQ(seq_action->index(), 2);
}

TEST(SequenceAction, FinishIfAnySucc_SuccHead) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool action_run_1 = false;
  bool action_run_2 = false;

  auto *seq_action = new SequenceAction(*loop);
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new FunctionAction(*loop,
    [&] {
      action_run_1 = true;
      return true;
    }
  ));
  seq_action->append(new FunctionAction(*loop,
    [&] {
      action_run_2 = true;
      return false;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_succ) {
      EXPECT_TRUE(is_succ);
      loop->exitLoop();
    }
  );
  seq_action->setFinishCondition(SequenceAction::FinishCondition::kAnySucc);
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_run_1);
  EXPECT_FALSE(action_run_2);
  EXPECT_EQ(seq_action->index(), 0);
}

TEST(SequenceAction, FinishIfAnySucc_SuccTail) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool action_run_1 = false;
  bool action_run_2 = false;

  auto *seq_action = new SequenceAction(*loop);
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new FunctionAction(*loop,
    [&] {
      action_run_1 = true;
      return false;
    }
  ));
  seq_action->append(new FunctionAction(*loop,
    [&] {
      EXPECT_TRUE(action_run_1);
      action_run_2 = true;
      return true;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_succ) {
      EXPECT_TRUE(is_succ);
      loop->exitLoop();
    }
  );
  seq_action->setFinishCondition(SequenceAction::FinishCondition::kAnySucc);
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_run_2);
  EXPECT_EQ(seq_action->index(), 1);
}

TEST(SequenceAction, FinishIfAllFinish_AllFail) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool action_run_1 = false;
  bool action_run_2 = false;

  auto *seq_action = new SequenceAction(*loop);
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new FunctionAction(*loop,
    [&] {
      action_run_1 = true;
      return false;
    }
  ));
  seq_action->append(new FunctionAction(*loop,
    [&] {
      EXPECT_TRUE(action_run_1);
      action_run_2 = true;
      return false;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_succ) {
      EXPECT_FALSE(is_succ);
      loop->exitLoop();
    }
  );
  seq_action->setFinishCondition(SequenceAction::FinishCondition::kAllFinish);
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_run_2);
  EXPECT_EQ(seq_action->index(), 2);
}

TEST(SequenceAction, FinishIfAllFinish_AllSucc) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool action_run_1 = false;
  bool action_run_2 = false;

  auto *seq_action = new SequenceAction(*loop);
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new FunctionAction(*loop,
    [&] {
      action_run_1 = true;
      return true;
    }
  ));
  seq_action->append(new FunctionAction(*loop,
    [&] {
      EXPECT_TRUE(action_run_1);
      action_run_2 = true;
      return true;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_succ) {
      EXPECT_TRUE(is_succ);
      loop->exitLoop();
    }
  );
  seq_action->setFinishCondition(SequenceAction::FinishCondition::kAllFinish);
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_run_2);
  EXPECT_EQ(seq_action->index(), 2);
}

}
}
