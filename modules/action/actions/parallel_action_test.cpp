#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "parallel_action.h"
#include "nondelay_action.h"
#include "sleep_action.h"
#include "../executor.h"

namespace tbox {
namespace action {

TEST(ParallelAction, TwoSleepAction) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  Executor exec(*loop);

  auto *para_action = new ParallelAction(exec.context());
  SetScopeExitAction([para_action] { delete para_action; });

  auto *sleep_action_1 = new SleepAction(exec.context(), std::chrono::milliseconds(300));
  auto *sleep_action_2 = new SleepAction(exec.context(), std::chrono::milliseconds(200));

  para_action->append(sleep_action_1);
  para_action->append(sleep_action_2);

  para_action->setFinishCallback(
    [loop](bool is_done) {
      EXPECT_TRUE(is_done);
      loop->exitLoop();
    }
  );

  auto start_time = std::chrono::steady_clock::now();
  para_action->start();
  loop->runLoop();

  auto d = std::chrono::steady_clock::now() - start_time;
  EXPECT_GT(d, std::chrono::milliseconds(290));
  EXPECT_LT(d, std::chrono::milliseconds(310));
}

}
}
