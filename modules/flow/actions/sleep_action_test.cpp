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

  SleepAction action(*loop);
  action.setDuration(std::chrono::milliseconds(10));
  action.init();

  bool is_finished = false;
  action.setFinishCallback(
    [loop, &is_finished] (bool is_succ) {
      loop->exitLoop();
      is_finished = true;
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

  SleepAction action_50ms(*loop);
  action_50ms.setDuration(std::chrono::milliseconds(50));
  action_50ms.setFinishCallback( [&] (bool is_succ) { ts_50ms = std::chrono::steady_clock::now(); });
  action_50ms.init();
  action_50ms.start();

  SleepAction action_100ms(*loop);
  action_100ms.setDuration(std::chrono::milliseconds(100));
  action_100ms.setFinishCallback( [&] (bool is_succ) { ts_100ms = std::chrono::steady_clock::now(); });
  action_100ms.init();
  action_100ms.start();

  SleepAction action_200ms(*loop);
  action_200ms.setDuration(std::chrono::milliseconds(200));
  action_200ms.setFinishCallback( [&] (bool is_succ) { ts_200ms = std::chrono::steady_clock::now(); });
  action_200ms.init();
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
  SleepAction action(*loop);
  action.setDuration(gen_time);
  action.setFinishCallback(
    [&] (bool succ) {
      EXPECT_TRUE(succ);
      end_ts = std::chrono::steady_clock::now();
      loop->exitLoop();
    }
  );
  action.init();
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

  auto gen_time = [&] { return std::chrono::milliseconds(time_tbl[index]); };

  auto cond_action = std::make_shared<FunctionAction>(*loop);
  cond_action->setFunc([&] { return index < NUMBER_OF_ARRAY(time_tbl); });
  cond_action->init();

  auto sleep_action = std::make_shared<SleepAction>(*loop);
  sleep_action->setDuration(gen_time);
  sleep_action->init();

  auto func_action = std::make_shared<FunctionAction>(*loop);
  func_action->setFunc([&] {
    ts.push_back(std::chrono::steady_clock::now());
    ++index;
    return true;
  });
  func_action->init();

  auto body_action = std::make_shared<SequenceAction>(*loop);
  body_action->append(sleep_action);
  body_action->append(func_action);
  body_action->init();

  LoopIfAction loop_if_action(*loop);
  loop_if_action.setIfAction(cond_action);
  loop_if_action.setExecAction(body_action);
  loop_if_action.setFinishCallback([=] (bool) { loop->exitLoop(); });
  loop_if_action.init();
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
