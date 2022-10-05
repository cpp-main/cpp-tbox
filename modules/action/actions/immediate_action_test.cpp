#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "immediate_action.h"
#include "../executor.h"

namespace tbox {
namespace action {

TEST(Action, ImmediateAction_True) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  Executor exec(*loop);
  ImmediateAction action(exec.context(), [] { return true; });
  bool is_callback = false;
  action.setFinishCallback(
    [&is_callback, loop] (bool is_done) {
      EXPECT_TRUE(is_done);
      is_callback = true;
      loop->exitLoop();
    }
  );
  action.start();

  loop->runLoop();
  EXPECT_TRUE(is_callback);
}

TEST(Action, ImmediateAction_False) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  Executor exec(*loop);
  ImmediateAction action(exec.context(), [] { return false; });
  bool is_callback = false;
  action.setFinishCallback(
    [&is_callback, loop] (bool is_done) {
      EXPECT_FALSE(is_done);
      is_callback = true;
      loop->exitLoop();
    }
  );
  action.start();

  loop->runLoop();
  EXPECT_TRUE(is_callback);
}

}
}
