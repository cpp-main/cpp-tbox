#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "invert_action.h"
#include "nondelay_action.h"

namespace tbox {
namespace action {

/**
 * return !false;
 */
TEST(InvertAction, _) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  auto nodelay_action = new NondelayAction(*loop, [] { return true; });
  InvertAction invert_action(*loop, nodelay_action);

  bool is_callback = false;
  invert_action.setFinishCallback(
    [&is_callback, loop] (bool is_succ) {
      EXPECT_FALSE(is_succ);
      is_callback = true;
      loop->exitLoop();
    }
  );
  invert_action.start();

  loop->runLoop();
  EXPECT_TRUE(is_callback);
}

}
}
