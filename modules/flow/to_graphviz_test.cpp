#include <iostream>
#include <gtest/gtest.h>
#include <tbox/base/json.hpp>
#include <tbox/base/log_output.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "to_graphviz.h"
#include "actions/function_action.h"
#include "actions/sequence_action.h"
#include "actions/if_else_action.h"
#include "actions/repeat_action.h"
#include "actions/succ_fail_action.h"

#include "state_machine.h"

namespace tbox {
namespace flow {

TEST(ToGraphviz, ActionJson) {
  LogOutput_Initialize();
  auto loop = event::Loop::New();
  SetScopeExitAction([loop] { delete loop; });

  auto seq_action = std::make_shared<SequenceAction>(*loop);
  seq_action->set_label("This is test");

  {
    auto if_else_action = std::make_shared<IfElseAction>(*loop);
    if_else_action->set_label("Just If-Else");

    {
      auto if_action = std::make_shared<SuccAction>(*loop);
      if_action->init();
      if_else_action->setIfAction(if_action);
    }

    {
      auto succ_action = std::make_shared<FunctionAction>(*loop);
      succ_action->setFunc([] {return true;});
      succ_action->init();
      if_else_action->setSuccAction(succ_action);
    }

    {
      auto fail_action = std::make_shared<FunctionAction>(*loop);
      fail_action->setFunc([] {return true;});
      fail_action->init();
      if_else_action->setFailAction(fail_action);
    }

    //if_else_action->init();
    seq_action->append(if_else_action);
  }

  {
    auto repeat_action = std::make_shared<RepeatAction>(*loop);
    {
      auto func_action = std::make_shared<FunctionAction>(*loop);
      func_action->setFunc([]{return true;});
      func_action->init();
      repeat_action->setChild(func_action);
    }
    repeat_action->setRepeatTimes(5);
    repeat_action->init();
    seq_action->append(repeat_action);
  }
  seq_action->init();
  seq_action->start();

  std::cout << ToGraphviz(seq_action.get());

  loop->exitLoop(std::chrono::milliseconds(10));
  loop->runLoop();
  LogOutput_Cleanup();
}

TEST(ToGraphviz, StateMachineJson)
{
  using SM = StateMachine;

  enum class StateId { kTerm, kInit, k1, k2 };
  enum class EventId { kTerm, k1, k2, k3 };

  SM sm, sub_sm;

  sm.newState(StateId::kInit, nullptr, nullptr, "Init");
  sm.newState(StateId::k1,    nullptr, nullptr, "State1");
  sm.newState(StateId::k2,    nullptr, nullptr, "State2");

  sm.addRoute(StateId::kInit, EventId::k1, StateId::k1,    nullptr, nullptr, "Event1");
  sm.addRoute(StateId::kInit, EventId::k2, StateId::k2,    nullptr, nullptr, "Event2");
  sm.addRoute(StateId::k1,    EventId::k1, StateId::kTerm, nullptr, nullptr, "Event1");
  sm.addRoute(StateId::k1,    EventId::k2, StateId::k2,    nullptr, nullptr, "Event2");
  sm.addRoute(StateId::k2,    EventId::k1, StateId::k1,    nullptr, nullptr, "Event1");
  sm.addRoute(StateId::k2,    EventId::k2, StateId::k1,    nullptr, nullptr, "Event2");
  sm.addEvent(StateId::k2,    EventId::k3, [](Event) { return -1; });

  sub_sm.newState(StateId::kInit, nullptr, nullptr, "Init");
  sub_sm.newState(StateId::k1,    nullptr, nullptr, "SubState1");
  sub_sm.newState(StateId::k2,    nullptr, nullptr, "SubState2");

  sub_sm.addRoute(StateId::kInit, EventId::k1, StateId::k1,    nullptr, nullptr, "Event1");
  sub_sm.addRoute(StateId::kInit, EventId::k2, StateId::k2,    nullptr, nullptr, "Event2");
  sub_sm.addRoute(StateId::k1,    EventId::k2, StateId::k2,    nullptr, nullptr, "Event2");
  sub_sm.addRoute(StateId::k1,    EventId::k1, StateId::kTerm, nullptr, nullptr, "Event1");
  sub_sm.addRoute(StateId::k2,    EventId::k1, StateId::k1,    nullptr, nullptr, "Event1");
  sub_sm.addRoute(StateId::k2,    EventId::k2, StateId::kTerm, nullptr, nullptr, "Event2");

  sm.setSubStateMachine(StateId::k1, &sub_sm);
  sm.start();
  sm.run(EventId::k1);

  std::cout << ToGraphviz(sm);
}

}
}
