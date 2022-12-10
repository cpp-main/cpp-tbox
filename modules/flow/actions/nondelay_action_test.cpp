#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "nondelay_action.h"

namespace tbox {
namespace flow {

TEST(NonDelayAction, True) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  NondelayAction action(*loop, [] { return true; });
  bool is_callback = false;
  action.setFinishCallback(
    [&is_callback, loop] (bool is_succ) {
      EXPECT_TRUE(is_succ);
      is_callback = true;
      loop->exitLoop();
    }
  );
  action.start();

  loop->runLoop();
  EXPECT_TRUE(is_callback);
}

TEST(NonDelayAction, False) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  NondelayAction action(*loop, [] { return false; });
  bool is_callback = false;
  action.setFinishCallback(
    [&is_callback, loop] (bool is_succ) {
      EXPECT_FALSE(is_succ);
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
