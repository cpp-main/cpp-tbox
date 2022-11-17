#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "action_executor.h"
#include "actions/nondelay_action.h"
#include "actions/sleep_action.h"

namespace tbox {
namespace action {

TEST(ActionExecutor, OneAction) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool is_run = false;
  bool is_action_started_cb_done = false;
  bool is_action_finished_cb_done = false;

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

  exec.setAllFinishedCallback([loop]{ loop->exitLoop(); });

  exec.append(new NondelayAction(*loop, [&]{ is_run = true; return true; }));

  loop->runLoop();
  EXPECT_TRUE(is_run);
  EXPECT_TRUE(is_action_started_cb_done);
  EXPECT_TRUE(is_action_finished_cb_done);
}

TEST(ActionExecutor, TwoActions) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  ActionExecutor exec;
  exec.setAllFinishedCallback([loop]{ loop->exitLoop(); });

  bool is_run_1 = false;
  bool is_run_2 = false;

  exec.append(
    new NondelayAction(*loop,
      [&]{
        is_run_1 = true;
        EXPECT_FALSE(is_run_2);
        return true;
      }
    )
  );
  exec.append(
    new NondelayAction(*loop,
      [&]{
        is_run_2 = true;
        EXPECT_TRUE(is_run_1);
        return true;
      }
    )
  );

  loop->runLoop();
  EXPECT_TRUE(is_run_1);
  EXPECT_TRUE(is_run_2);
}

TEST(ActionExecutor, CancelAction)
{
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  ActionExecutor exec;
  exec.setAllFinishedCallback([loop]{ loop->exitLoop(); });

  bool is_run_1 = false;
  bool is_run_2 = false;
  bool is_run_3 = false;

  exec.append(
    new NondelayAction(*loop,
      [&]{
        is_run_1 = true;
        EXPECT_FALSE(is_run_2);
        EXPECT_FALSE(is_run_3);
        return true;
      }
    )
  );
  exec.append(
    new NondelayAction(*loop,
      [&]{
        is_run_2 = true;
        return true;
      }
    )
  );
  exec.append(
    new NondelayAction(*loop,
      [&]{
        is_run_3 = true;
        EXPECT_TRUE(is_run_1);
        return true;
      }
    )
  );

  exec.cancel(2);

  loop->runLoop();
  EXPECT_TRUE(is_run_1);
  EXPECT_FALSE(is_run_2);
  EXPECT_TRUE(is_run_3);
}

TEST(ActionExecutor, Prio)
{
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  ActionExecutor exec;
  exec.setAllFinishedCallback([loop]{ loop->exitLoop(); });

  int index = 0;

  exec.append(
    new SleepAction(*loop, std::chrono::milliseconds(10)),
    1
  );

  exec.append(
    new NondelayAction(*loop,
      [&]{
        EXPECT_EQ(index, 3);
        ++index;
        return true;
      }
    ),
    2
  );

  exec.append(
    new NondelayAction(*loop,
      [&]{
        EXPECT_EQ(index, 1);
        ++index;
        return true;
      }
    ),
    1
  );
  exec.append(
    new NondelayAction(*loop,
      [&]{
        EXPECT_EQ(index, 2);
        ++index;
        return true;
      }
    ),
    1
  );

  exec.append(
    new NondelayAction(*loop,
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
}

}
}
