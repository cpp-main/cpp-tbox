#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "sleep_action.h"

namespace tbox {
namespace action {

//! 检查任务有无执行
TEST(SleepAction, Basic) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  SleepAction action(*loop, std::chrono::milliseconds(10));
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

  SleepAction action_50ms(*loop, std::chrono::milliseconds(50));
  SleepAction action_100ms(*loop, std::chrono::milliseconds(100));
  SleepAction action_200ms(*loop, std::chrono::milliseconds(200));

  action_50ms.setFinishCallback( [&] (bool is_succ) { ts_50ms = std::chrono::steady_clock::now(); });
  action_100ms.setFinishCallback( [&] (bool is_succ) { ts_100ms = std::chrono::steady_clock::now(); });
  action_200ms.setFinishCallback( [&] (bool is_succ) { ts_200ms = std::chrono::steady_clock::now(); });

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

}
}
