#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "sequence_action.h"
#include "immediate_action.h"
#include "../executor.h"

namespace tbox {
namespace action {

TEST(SequenceAction, AllSucc) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  Executor exec(*loop);

  bool action_done_1 = false;
  bool action_done_2 = false;

  auto *seq_action = new SequenceAction(exec.context());
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new ImmediateAction(exec.context(),
    [&] {
      action_done_1 = true; 
      return true;
    }
  ));
  seq_action->append(new ImmediateAction(exec.context(),
    [&] {
      EXPECT_TRUE(action_done_1);
      action_done_2 = true; 
      return true;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_done) {
      EXPECT_TRUE(is_done);
      loop->exitLoop();
    }
  );
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_done_2);
  EXPECT_EQ(seq_action->failAt(), -1);
}

TEST(SequenceAction, FailHead) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  Executor exec(*loop);

  bool action_done_1 = false;
  bool action_done_2 = false;

  auto *seq_action = new SequenceAction(exec.context());
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new ImmediateAction(exec.context(),
    [&] {
      action_done_1 = true; 
      return false;
    }
  ));
  seq_action->append(new ImmediateAction(exec.context(),
    [&] {
      action_done_2 = true; 
      return false;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_done) {
      EXPECT_FALSE(is_done);
      loop->exitLoop();
    }
  );
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_done_1);
  EXPECT_FALSE(action_done_2);
  EXPECT_EQ(seq_action->failAt(), 0);
}


TEST(SequenceAction, FailTail) {
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  Executor exec(*loop);

  bool action_done_1 = false;
  bool action_done_2 = false;

  auto *seq_action = new SequenceAction(exec.context());
  SetScopeExitAction([seq_action] { delete seq_action; });

  seq_action->append(new ImmediateAction(exec.context(),
    [&] {
      action_done_1 = true; 
      return true;
    }
  ));
  seq_action->append(new ImmediateAction(exec.context(),
    [&] {
      EXPECT_TRUE(action_done_1);
      action_done_2 = true; 
      return false;
    }
  ));
  seq_action->setFinishCallback(
    [loop](bool is_done) {
      EXPECT_FALSE(is_done);
      loop->exitLoop();
    }
  );
  seq_action->start();

  loop->runLoop();
  EXPECT_TRUE(action_done_2);
  EXPECT_EQ(seq_action->failAt(), 1);
}


}
}
