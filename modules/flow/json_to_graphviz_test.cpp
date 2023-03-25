#include <iostream>
#include <gtest/gtest.h>
#include <tbox/base/json.hpp>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "json_to_graphviz.h"
#include "actions/function_action.h"
#include "actions/sequence_action.h"
#include "actions/if_else_action.h"
#include "actions/repeat_action.h"
#include "actions/succ_fail_action.h"

namespace tbox {
namespace flow {

TEST(JsonToGraphviz, ActionJson) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    auto seq_action = new SequenceAction(*loop);
    auto if_else_action = new IfElseAction(*loop,
            new SuccAction(*loop),
            new FunctionAction(*loop, []{return true;}),
            new FunctionAction(*loop, []{return true;}));
    seq_action->append(if_else_action);
    auto repeat_action = new RepeatAction(*loop,
            new FunctionAction(*loop, []{return true;}), 5);
    seq_action->append(repeat_action);
    seq_action->start();

    Json js;
    seq_action->toJson(js);
    std::cout << ActionJsonToGraphviz(js);

    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    delete seq_action;
}

}
}
