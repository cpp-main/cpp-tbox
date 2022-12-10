#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "fallback_action.h"
#include "nondelay_action.h"
#include "sleep_action.h"

namespace tbox {
namespace flow {

TEST(FallbackAction, AllFail) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool action_run_1 = false;
  bool action_run_2 = false;

  auto *seq_action = new FallbackAction(*loop);
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new NondelayAction(*loop,
    [&] {
      action_run_1 = true;
      return false;
    }
  ));
  seq_action->append(new NondelayAction(*loop,
    [&] {
      EXPECT_TRUE(action_run_1);
      action_run_2 = true;
      return false;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_succ) {
      EXPECT_FALSE(is_succ);
      loop->exitLoop();
    }
  );
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_run_2);
  EXPECT_EQ(seq_action->index(), 2);
}

TEST(FallbackAction, SuccHead) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool action_run_1 = false;
  bool action_run_2 = false;

  auto *seq_action = new FallbackAction(*loop);
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new NondelayAction(*loop,
    [&] {
      action_run_1 = true;
      return true;
    }
  ));
  seq_action->append(new NondelayAction(*loop,
    [&] {
      action_run_2 = true;
      return false;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_succ) {
      EXPECT_TRUE(is_succ);
      loop->exitLoop();
    }
  );
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_run_1);
  EXPECT_FALSE(action_run_2);
  EXPECT_EQ(seq_action->index(), 0);
}


TEST(FallbackAction, SuccTail) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  bool action_run_1 = false;
  bool action_run_2 = false;

  auto *seq_action = new FallbackAction(*loop);
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new NondelayAction(*loop,
    [&] {
      action_run_1 = true;
      return false;
    }
  ));
  seq_action->append(new NondelayAction(*loop,
    [&] {
      EXPECT_TRUE(action_run_1);
      action_run_2 = true;
      return true;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_succ) {
      EXPECT_TRUE(is_succ);
      loop->exitLoop();
    }
  );
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_run_2);
  EXPECT_EQ(seq_action->index(), 1);
}

}
}
