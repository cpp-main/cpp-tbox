/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <iostream>
#include <gtest/gtest.h>
#include <tbox/base/json.hpp>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "to_graphviz.h"
#include "actions/function_action.h"
#include "actions/sequence_action.h"
#include "actions/if_else_action.h"
#include "actions/repeat_action.h"
#include "actions/succ_fail_action.h"
#include "actions/switch_action.h"
#include "actions/sleep_action.h"
#include "actions/dummy_action.h"

#include "state_machine.h"

namespace tbox {
namespace flow {

TEST(ToGraphviz, ActionJson) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    auto seq_action = new SequenceAction(*loop);
    seq_action->set_label("This is test");
    seq_action->vars().define("seq.value", 12);
    {
        auto if_else_action = new IfElseAction(*loop);
        if_else_action->vars().define("if_else.name", "hello");
        if_else_action->vars().define("if_else.value", 100);
        if_else_action->setChildAs(new SuccAction(*loop), "if");
        if_else_action->setChildAs(new FunctionAction(*loop, []{return true;}), "succ");
        if_else_action->setChildAs(new FunctionAction(*loop, []{return true;}), "fail");
        if_else_action->set_label("Just If-Else");
        seq_action->addChild(if_else_action);
    }
    {
        auto repeat_action = new RepeatAction(*loop, 5);
        repeat_action->setChild(new FunctionAction(*loop, []{return true;}));
        seq_action->addChild(repeat_action);
    }
    {
        auto action = new SwitchAction(*loop);
        seq_action->addChild(action);
        auto switch_action = new FunctionAction(*loop, [] { return true; });
        auto case_a_action = new SleepAction(*loop, std::chrono::seconds(1));
        auto case_b_action = new DummyAction(*loop);
        auto default_action = new SuccAction(*loop);

        action->setChildAs(switch_action, "switch");
        action->setChildAs(case_a_action, "case:a");
        action->setChildAs(case_b_action, "case:b");
        action->setChildAs(default_action, "default");
    }

    seq_action->start();

    std::cout << ToGraphviz(seq_action);

    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    delete seq_action;
}

TEST(ToGraphviz, StateMachineJson)
{
    using SM = StateMachine;

    enum class StateId { kTerm, kInit, k1, k2 };
    enum class EventId { kAny, k1, k2, k3 };

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
