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
#include <tbox/event/timer_event.h>
#include <tbox/base/scope_exit.hpp>

#include "action_executor.h"
#include "actions/function_action.h"
#include "actions/sleep_action.h"

namespace tbox {
namespace flow {

TEST(ActionExecutor, OneAction) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool is_run = false;
  bool is_action_started_cb_done = false;
  bool is_action_finished_cb_done = false;
  bool is_all_action_finished_cb_done = false;

  ActionExecutor exec;
  exec.setActionStartedCallback(
    [&] (ActionExecutor::ActionId id) {
      EXPECT_EQ(id, 1);
      is_action_started_cb_done = true;
    }
  );

  exec.setActionFinishedCallback(
    [&] (ActionExecutor::ActionId id) {
      EXPECT_EQ(id, 1);
      is_action_finished_cb_done = true;
    }
  );

  exec.setAllFinishedCallback([&]{ is_all_action_finished_cb_done = true; });

  exec.append(new FunctionAction(*loop, [&]{ is_run = true; return true; }));

  loop->exitLoop(std::chrono::milliseconds(5));
  loop->runLoop();
  EXPECT_TRUE(is_run);
  EXPECT_TRUE(is_action_started_cb_done);
  EXPECT_TRUE(is_action_finished_cb_done);
  EXPECT_TRUE(is_all_action_finished_cb_done);
}

TEST(ActionExecutor, TwoActions) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool is_all_action_finished = false;

  ActionExecutor exec;
  exec.setAllFinishedCallback([&]{ is_all_action_finished = true; });

  bool is_run_1 = false;
  bool is_run_2 = false;

  exec.append(
    new FunctionAction(*loop,
      [&]{
        is_run_1 = true;
        EXPECT_FALSE(is_run_2);
        return true;
      }
    )
  );
  exec.append(
    new FunctionAction(*loop,
      [&]{
        is_run_2 = true;
        EXPECT_TRUE(is_run_1);
        return true;
      }
    )
  );

  loop->exitLoop(std::chrono::milliseconds(5));
  loop->runLoop();

  EXPECT_TRUE(is_run_1);
  EXPECT_TRUE(is_run_2);
}

TEST(ActionExecutor, CancelAction)
{
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool is_all_action_finished = false;

  ActionExecutor exec;
  exec.setAllFinishedCallback([&]{ is_all_action_finished = true; });

  bool is_run_1 = false;
  bool is_run_2 = false;
  bool is_run_3 = false;

  exec.append(new SleepAction(*loop, std::chrono::milliseconds(1)));

  exec.append(
    new FunctionAction(*loop,
      [&]{
        is_run_1 = true;
        EXPECT_FALSE(is_run_2);
        EXPECT_FALSE(is_run_3);
        return true;
      }
    )
  );
  auto tobe_cancel_action = exec.append(
    new FunctionAction(*loop,
      [&]{
        is_run_2 = true;
        return true;
      }
    )
  );
  exec.append(
    new FunctionAction(*loop,
      [&]{
        is_run_3 = true;
        EXPECT_TRUE(is_run_1);
        return true;
      }
    )
  );

  exec.cancel(tobe_cancel_action);

  loop->exitLoop(std::chrono::milliseconds(5));
  loop->runLoop();
  EXPECT_TRUE(is_run_1);
  EXPECT_FALSE(is_run_2);
  EXPECT_TRUE(is_run_3);
  EXPECT_TRUE(is_all_action_finished);
}

/**
 * 优先级测试
 *
 * 1.先加入一个10ms的普通延时动作1
 * 2.再加入一个低优先级的动作2；
 * 3.再加入两个中优先级的动作3,4
 * 4.最后加入一个中高先级的动作5
 * 它的执行顺序应该是5->1->3->4->2
 */
TEST(ActionExecutor, Prio)
{
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  ActionExecutor exec;

  std::vector<ActionExecutor::ActionId> start_seq;  //! 启动顺序
  std::vector<ActionExecutor::ActionId> finish_seq; //! 结束顺序

  exec.setAllFinishedCallback([loop]{ loop->exitLoop(); });
  exec.setActionStartedCallback(
    [&] (ActionExecutor::ActionId id) { start_seq.push_back(id); }
  );
  exec.setActionFinishedCallback(
    [&] (ActionExecutor::ActionId id) { finish_seq.push_back(id); }
  );

  int index = 0;

  //! 加入延时动作1，防止立即开始执行
  exec.append(
    new SleepAction(*loop, std::chrono::milliseconds(10)),
    1
  );

  //! 加入低优先级动作2
  exec.append(
    new FunctionAction(*loop,
      [&]{
        EXPECT_EQ(index, 3);
        ++index;
        return true;
      }
    ),
    2
  );

  //! 加入中优先级动作3
  exec.append(
    new FunctionAction(*loop,
      [&]{
        EXPECT_EQ(index, 1);
        ++index;
        return true;
      }
    ),
    1
  );
  //! 加入中优先级动作4
  exec.append(
    new FunctionAction(*loop,
      [&]{
        EXPECT_EQ(index, 2);
        ++index;
        return true;
      }
    ),
    1
  );

  //! 加入高优先级动作5
  exec.append(
    new FunctionAction(*loop,
      [&]{
        EXPECT_EQ(index, 0);
        ++index;
        return true;
      }
    ),
    0
  );

  loop->runLoop();
  EXPECT_EQ(index, 4);

  ASSERT_EQ(start_seq.size(), 5);
  ASSERT_EQ(finish_seq.size(), 5);

  //! 启动顺序：1->5->3->4->2
  EXPECT_EQ(start_seq[0], 1);
  EXPECT_EQ(start_seq[1], 5);
  EXPECT_EQ(start_seq[2], 3);
  EXPECT_EQ(start_seq[3], 4);
  EXPECT_EQ(start_seq[4], 2);

  //! 结束顺序：5->1->3->4->2
  //! 任务1被任务5抢点了
  EXPECT_EQ(finish_seq[0], 5);
  EXPECT_EQ(finish_seq[1], 1);
  EXPECT_EQ(finish_seq[2], 3);
  EXPECT_EQ(finish_seq[3], 4);
  EXPECT_EQ(finish_seq[4], 2);
}

/**
 * 测试运行过程中取消当前任务
 *
 * 1.加入一个1秒的延迟；
 * 2.加入一个FunctionAction，在该动作中记录执行时间点exec_ts；
 * 3.创建一个定时器，20ms后触发；
 * 4.在执行loop之前，记录start_ts
 * 5.检查 start_ts 到 exec_ts 的时间差，正常情况下就在20ms左右
 */
TEST(ActionExecutor, CancelCurrent) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  ActionExecutor exec;

  exec.setAllFinishedCallback([loop]{ loop->exitLoop(); });

  std::chrono::steady_clock::time_point start_ts;
  std::chrono::steady_clock::time_point exec_ts;
  bool is_run = false;

  exec.append(new SleepAction(*loop, std::chrono::seconds(1)));
  exec.append(
    new FunctionAction(*loop,
      [&]{
        is_run = true;
        exec_ts = std::chrono::steady_clock::now();
        return true;
      }
    )
  );

  auto timer = loop->newTimerEvent();
  SetScopeExitAction([timer] { delete timer; });
  timer->initialize(std::chrono::milliseconds(20), tbox::event::Event::Mode::kOneshot);
  timer->setCallback([&exec] { exec.cancelCurrent(); });
  timer->enable();

  start_ts = std::chrono::steady_clock::now();
  loop->runLoop();

  EXPECT_TRUE(is_run);
  auto count = std::chrono::duration_cast<std::chrono::microseconds>(exec_ts - start_ts).count();
  EXPECT_GE(count, 19000);
  EXPECT_LE(count, 21000);
}

}
}
