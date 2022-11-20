#include <vector>

#include <gtest/gtest.h>
#include <tbox/base/defines.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "dynamic_sleep_action.h"
#include "loop_action.h"
#include "nondelay_action.h"
#include "sequence_action.h"

namespace tbox {
namespace action {

TEST(DynamicSleepAction, OneAction) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  auto start_ts = std::chrono::steady_clock::now();
  auto end_ts = start_ts;

  auto gen_time = [] { return std::chrono::milliseconds(50); };
  DynamicSleepAction action(*loop, gen_time);
  action.setFinishCallback(
    [&] (bool succ) {
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
TEST(DynamicSleepAction, LoopAction) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  int time_tbl[] = { 50, 100, 200, 150, 10 };
  size_t index = 0;

  std::vector<std::chrono::steady_clock::time_point> ts;
  ts.push_back(std::chrono::steady_clock::now());

  auto gen_time = [&] { return std::chrono::milliseconds(time_tbl[index]); };
  auto seq_action = new SequenceAction(*loop);

  seq_action->append(new DynamicSleepAction(*loop, gen_time));
  seq_action->append(
    new NondelayAction(*loop,
      [&] {
        ts.push_back(std::chrono::steady_clock::now());
        ++index;
        return index < NUMBER_OF_ARRAY(time_tbl);
      }
    )
  );
  LoopAction loop_action(*loop, seq_action, LoopAction::Mode::kUntilFail);
  loop_action.setFinishCallback([=] (bool) { loop->exitLoop(); });
  loop_action.start();

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
