#include <iostream>
#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "loop_action.h"
#include "nondelay_action.h"
#include "../executor.h"

namespace tbox {
namespace action {

/**
 * return !false;
 */
TEST(LoopAction, Forever) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  Executor exec(*loop);
  int loop_times = 0;
  auto nondelay_action = new NondelayAction(exec.context(), "",
    [&] {
      std::cout << '.' << std::endl;
      ++loop_times;
      return true;
    }
  );
  LoopAction loop_action(exec.context(), "", nondelay_action, LoopAction::Mode::kForever);
  bool is_finished = false;
  loop_action.setFinishCallback([&] (bool) { is_finished = true; });

  loop_action.start();
  loop->exitLoop(std::chrono::milliseconds(1000));
  loop->runLoop();
  loop_action.stop();
  std::cout << "loop_times:" << loop_times << std::endl;
  EXPECT_FALSE(is_finished);
}

}
}
