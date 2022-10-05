#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "sleep_action.h"
#include "../executor.h"

namespace tbox {
namespace action {

TEST(Action, SequenceAction) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  Executor exec(*loop);
  //loop->runLoop();
}
}
}
