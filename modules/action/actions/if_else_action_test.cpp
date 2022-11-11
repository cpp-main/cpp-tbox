#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "if_else_action.h"
#include "nondelay_action.h"

namespace tbox {
namespace action {

TEST(IfElseAction, CondSucc) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool cond_action_run = false;
  bool if_action_run = false;
  bool else_action_run = false;
  bool if_else_action_run = false;

  auto cond_action = new NondelayAction(*loop, [&] { cond_action_run = true; return true; });
  auto if_action = new NondelayAction(*loop, [&] { if_action_run = true; return true; });
  auto else_action = new NondelayAction(*loop, [&] { else_action_run = true; return true; });

  IfElseAction if_else_action(*loop, cond_action, if_action, else_action);

  if_else_action.setFinishCallback(
    [&] (bool is_succ) {
      EXPECT_TRUE(is_succ);
      if_else_action_run = true;
      loop->exitLoop();
    }
  );
  if_else_action.start();

  loop->runLoop();
  EXPECT_TRUE(cond_action_run);
  EXPECT_TRUE(if_else_action_run);
  EXPECT_TRUE(if_action_run);
  EXPECT_FALSE(else_action_run);
}

TEST(IfElseAction, CondFail) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool cond_action_run = false;
  bool if_action_run = false;
  bool else_action_run = false;
  bool if_else_action_run = false;

  auto cond_action = new NondelayAction(*loop, [&] { cond_action_run = true; return false; });
  auto if_action = new NondelayAction(*loop, [&] { if_action_run = true; return true; });
  auto else_action = new NondelayAction(*loop, [&] { else_action_run = true; return true; });

  IfElseAction if_else_action(*loop, cond_action, if_action, else_action);

  if_else_action.setFinishCallback(
    [&] (bool is_succ) {
      EXPECT_TRUE(is_succ);
      if_else_action_run = true;
      loop->exitLoop();
    }
  );
  if_else_action.start();

  loop->runLoop();
  EXPECT_TRUE(cond_action_run);
  EXPECT_TRUE(if_else_action_run);
  EXPECT_FALSE(if_action_run);
  EXPECT_TRUE(else_action_run);
}

TEST(IfElseAction, CondSuccNoIfAction) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool cond_action_run = false;
  bool else_action_run = false;
  bool if_else_action_run = false;

  auto cond_action = new NondelayAction(*loop, [&] { cond_action_run = true; return true; });
  auto else_action = new NondelayAction(*loop, [&] { else_action_run = true; return true; });

  IfElseAction if_else_action(*loop, cond_action, nullptr, else_action);

  if_else_action.setFinishCallback(
    [&] (bool is_succ) {
      EXPECT_TRUE(is_succ);
      if_else_action_run = true;
      loop->exitLoop();
    }
  );
  if_else_action.start();

  loop->runLoop();
  EXPECT_TRUE(cond_action_run);
  EXPECT_TRUE(if_else_action_run);
  EXPECT_FALSE(else_action_run);
}

TEST(IfElseAction, CondFailNoElseAction) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool cond_action_run = false;
  bool if_action_run = false;
  bool if_else_action_run = false;

  auto cond_action = new NondelayAction(*loop, [&] { cond_action_run = true; return false; });
  auto if_action = new NondelayAction(*loop, [&] { if_action_run = true; return true; });

  IfElseAction if_else_action(*loop, cond_action, if_action, nullptr);

  if_else_action.setFinishCallback(
    [&] (bool is_succ) {
      EXPECT_TRUE(is_succ);
      if_else_action_run = true;
      loop->exitLoop();
    }
  );
  if_else_action.start();

  loop->runLoop();
  EXPECT_TRUE(cond_action_run);
  EXPECT_TRUE(if_else_action_run);
  EXPECT_FALSE(if_action_run);
}

}
}
