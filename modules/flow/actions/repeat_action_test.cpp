#include <iostream>
#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "repeat_action.h"
#include "function_action.h"

namespace tbox {
namespace flow {

/**
 *  int loop_times = 0;
 *  for (int i = 0; i < 100; ++i) {
 *    ++loop_times;
 *  }
 */
TEST(RepeatAction, FunctionActionRepeat3NoBreak) {
  auto loop = event::Loop::New();

  int loop_times = 0;
  auto function_action = new FunctionAction(*loop,
    [&] {
      ++loop_times;
      return true;
    }
  );
  RepeatAction repeat_action(*loop, function_action, 3, RepeatAction::Mode::kNoBreak);
  bool is_finished = false;
  repeat_action.setFinishCallback([&] (bool) { is_finished = true; });

  repeat_action.start();
  loop->exitLoop(std::chrono::milliseconds(10));
  loop->runLoop();

  delete loop;

  EXPECT_TRUE(is_finished);
  EXPECT_EQ(loop_times, 3);
  EXPECT_EQ(repeat_action.state(), Action::State::kFinished);
}

/**
 *  int loop_times = 0;
 *  while (true) {
 *    ++loop_times;
 *  }
 */
TEST(RepeatAction, FunctionActionForeverNoBreak) {
  auto loop = event::Loop::New();

  int loop_times = 0;
  auto function_action = new FunctionAction(*loop,
    [&] {
      ++loop_times;
      return true;
    }
  );
  RepeatAction repeat_action(*loop, function_action, 0, RepeatAction::Mode::kNoBreak);
  bool is_finished = false;
  repeat_action.setFinishCallback([&] (bool) { is_finished = true; });

  repeat_action.start();
  loop->exitLoop(std::chrono::milliseconds(1000));
  loop->runLoop();

  repeat_action.stop();

  delete loop;

  EXPECT_FALSE(is_finished);
  EXPECT_GT(loop_times, 1000);
  EXPECT_EQ(repeat_action.state(), Action::State::kStoped);
}

/**
 *  int loop_times = 0;
 *  for (int i = 0; i < 5; ++i) {
 *    ++loop_times;
 *    if (loop_times >= 3)
 *      return true;
 *  }
 */
TEST(RepeatAction, FunctionActionRepeat5BreakFail) {
  auto loop = event::Loop::New();

  int loop_times = 0;
  auto function_action = new FunctionAction(*loop,
    [&] {
      ++loop_times;
      return loop_times < 3;
    }
  );
  RepeatAction repeat_action(*loop, function_action, 5, RepeatAction::Mode::kBreakFail);
  bool is_finished = false;
  repeat_action.setFinishCallback(
    [&] (bool is_succ) {
      EXPECT_FALSE(is_succ);
      is_finished = true;
    }
  );

  repeat_action.start();
  loop->exitLoop(std::chrono::milliseconds(10));
  loop->runLoop();

  delete loop;

  EXPECT_TRUE(is_finished);
  EXPECT_EQ(loop_times, 3);
  EXPECT_EQ(repeat_action.state(), Action::State::kFinished);
}

/**
 *  int loop_times = 0;
 *  for (int i = 0; i < 5; ++i) {
 *    ++loop_times;
 *    if (loop_times < 3)
 *      return true;
 *  }
 */
TEST(RepeatAction, FunctionActionRepeat5BreakSucc) {
  auto loop = event::Loop::New();

  int loop_times = 0;
  auto function_action = new FunctionAction(*loop,
    [&] {
      ++loop_times;
      return loop_times >= 3;
    }
  );
  RepeatAction repeat_action(*loop, function_action, 5, RepeatAction::Mode::kBreakSucc);
  bool is_finished = false;
  repeat_action.setFinishCallback(
    [&] (bool is_succ) {
      EXPECT_TRUE(is_succ);
      is_finished = true;
    }
  );

  repeat_action.start();
  loop->exitLoop(std::chrono::milliseconds(10));
  loop->runLoop();

  delete loop;

  EXPECT_TRUE(is_finished);
  EXPECT_EQ(loop_times, 3);
  EXPECT_EQ(repeat_action.state(), Action::State::kFinished);
}

}
}
