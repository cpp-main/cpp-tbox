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
#include <tbox/base/log_output.h>
#include <tbox/base/log.h>

#include "sleep_action.h"
#include "loop_if_action.h"
#include "function_action.h"
#include "sequence_action.h"

namespace tbox {
namespace flow {

//! 检查任务有无执行
TEST(SleepAction, Basic) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  SleepAction action(*loop, std::chrono::milliseconds(10));
  bool is_finished = false;
  action.setFinishCallback(
    [loop, &is_finished] (bool is_succ, const Action::Reason&, const Action::Trace&) {
      loop->exitLoop();
      is_finished = true;
      (void)is_succ;
    }
  );
  action.start();

  loop->exitLoop(std::chrono::milliseconds(20));
  loop->runLoop();
  EXPECT_TRUE(is_finished);
}

//! 创建三个延时任务，令它们同时启动，检查完成的时间点是否与设置时长一致
TEST(SleepAction, Accuracy) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  std::chrono::steady_clock::time_point ts_start;
  std::chrono::steady_clock::time_point ts_50ms;
  std::chrono::steady_clock::time_point ts_100ms;
  std::chrono::steady_clock::time_point ts_200ms;

  SleepAction action_50ms(*loop, std::chrono::milliseconds(50));
  SleepAction action_100ms(*loop, std::chrono::milliseconds(100));
  SleepAction action_200ms(*loop, std::chrono::milliseconds(200));

  action_50ms.setFinishCallback( [&] (bool is_succ, const Action::Reason&, const Action::Trace&)
    { ts_50ms = std::chrono::steady_clock::now(); (void)is_succ; });
  action_100ms.setFinishCallback( [&] (bool is_succ, const Action::Reason&, const Action::Trace&)
    { ts_100ms = std::chrono::steady_clock::now(); (void)is_succ; });
  action_200ms.setFinishCallback( [&] (bool is_succ, const Action::Reason&, const Action::Trace&)
    { ts_200ms = std::chrono::steady_clock::now(); (void)is_succ; });

  action_50ms.start();
  action_100ms.start();
  action_200ms.start();

  ts_start = std::chrono::steady_clock::now();
  loop->exitLoop(std::chrono::milliseconds(250));
  loop->runLoop();

  auto cout_50 = std::chrono::duration_cast<std::chrono::milliseconds>(ts_50ms - ts_start).count();
  EXPECT_LE(cout_50, 51);
  EXPECT_GE(cout_50, 49);
  auto cout_100 = std::chrono::duration_cast<std::chrono::milliseconds>(ts_100ms - ts_start).count();
  EXPECT_LE(cout_100, 101);
  EXPECT_GE(cout_100, 99);
  auto cout_200 = std::chrono::duration_cast<std::chrono::milliseconds>(ts_200ms - ts_start).count();
  EXPECT_LE(cout_200, 201);
  EXPECT_GE(cout_200, 199);
}

TEST(SleepAction, GenOneAction) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  auto start_ts = std::chrono::steady_clock::now();
  auto end_ts = start_ts;

  auto gen_time = [] { return std::chrono::milliseconds(50); };
  SleepAction action(*loop, gen_time);
  action.setFinishCallback(
    [&] (bool succ, const Action::Reason&, const Action::Trace&) {
      EXPECT_TRUE(succ);
      end_ts = std::chrono::steady_clock::now();
      loop->exitLoop();
    }
  );
  action.start();

  loop->runLoop();

  auto millisec_count = std::chrono::duration_cast<std::chrono::milliseconds>(end_ts - start_ts).count();
  EXPECT_LE(millisec_count, 51);
  EXPECT_GE(millisec_count, 49);
}

//  测试以下逻辑
//
//  int time_tbl[] = { 50, 100, 200, 150, 10 };
//  size_t index = 0;
//  std::vector<std::chrono::steady_clock::time_point> ts;
//
//  ts.push_back(std::chrono::steady_clock::now());
//  while (index < NUMBER_OF_ARRAY(time_tbl)) {
//    Sleep(time_tbl[index]);
//    ts.push_back(std::chrono::steady_clock::now());
//    ++index;
//  }
//
//  检查 ts 中相邻两个之间的时间差是否与 time_tbl 一致
//
TEST(SleepAction, GenLoopAction) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    int time_tbl[] = { 50, 100, 200, 150, 10 };
    size_t index = 0;

    std::vector<std::chrono::steady_clock::time_point> ts;
    ts.push_back(std::chrono::steady_clock::now());

    LoopIfAction loop_if_action(*loop);

    auto gen_time = [&] { return std::chrono::milliseconds(time_tbl[index]); };
    auto cond_action = new FunctionAction(*loop, [&] { return index < NUMBER_OF_ARRAY(time_tbl); });
    auto body_action = new SequenceAction(*loop);

    body_action->addChild(new SleepAction(*loop, gen_time));
    body_action->addChild(
        new FunctionAction(*loop,
            [&] {
                ts.push_back(std::chrono::steady_clock::now());
                ++index;
                return true;
            }
        )
    );
    EXPECT_TRUE(loop_if_action.setChildAs(cond_action, "if"));
    EXPECT_TRUE(loop_if_action.setChildAs(body_action, "exec"));
    EXPECT_TRUE(loop_if_action.isReady());
    loop_if_action.setFinishCallback([=] (bool, const Action::Reason&, const Action::Trace&) { loop->exitLoop(); });
    loop_if_action.start();

    loop->runLoop();

    ASSERT_EQ(ts.size(), NUMBER_OF_ARRAY(time_tbl) + 1);
    for (size_t i = 0; i < NUMBER_OF_ARRAY(time_tbl); ++i) {
        int duration = time_tbl[i];
        int count = std::chrono::duration_cast<std::chrono::milliseconds>(ts[i+1] - ts[i]).count();
        EXPECT_LE(count, duration + 1);
        EXPECT_GE(count, duration - 1);
    }
}

}
}
