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
#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "if_else_action.h"
#include "function_action.h"
#include "succ_fail_action.h"

namespace tbox {
namespace flow {

TEST(IfElseAction, CondSucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    IfElseAction if_else_action(*loop);

    bool cond_action_run = false;
    bool if_action_run = false;
    bool else_action_run = false;
    bool if_else_action_run = false;

    auto cond_action = new FunctionAction(*loop, [&] { cond_action_run = true; return true; });
    auto if_action = new FunctionAction(*loop, [&] { if_action_run = true; return true; });
    auto else_action = new FunctionAction(*loop, [&] { else_action_run = true; return true; });

    EXPECT_TRUE(if_else_action.setChildAs(cond_action, "if"));
    EXPECT_TRUE(if_else_action.setChildAs(if_action, "succ"));
    EXPECT_TRUE(if_else_action.setChildAs(else_action, "fail"));
    EXPECT_TRUE(if_else_action.isReady());

    if_else_action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            if_else_action_run = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(if_else_action.isReady());
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

    IfElseAction if_else_action(*loop);

    bool cond_action_run = false;
    bool if_action_run = false;
    bool else_action_run = false;
    bool if_else_action_run = false;

    auto cond_action = new FunctionAction(*loop, [&] { cond_action_run = true; return false; });
    auto if_action = new FunctionAction(*loop, [&] { if_action_run = true; return true; });
    auto else_action = new FunctionAction(*loop, [&] { else_action_run = true; return true; });

    EXPECT_TRUE(if_else_action.setChildAs(cond_action, "if"));
    EXPECT_TRUE(if_else_action.setChildAs(if_action, "succ"));
    EXPECT_TRUE(if_else_action.setChildAs(else_action, "fail"));
    EXPECT_TRUE(if_else_action.isReady());

    if_else_action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &, const Action::Trace &) {
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

    IfElseAction if_else_action(*loop);

    bool cond_action_run = false;
    bool else_action_run = false;
    bool if_else_action_run = false;

    auto cond_action = new FunctionAction(*loop, [&] { cond_action_run = true; return true; });
    auto else_action = new FunctionAction(*loop, [&] { else_action_run = true; return true; });

    EXPECT_TRUE(if_else_action.setChildAs(cond_action, "if"));
    EXPECT_TRUE(if_else_action.setChildAs(else_action, "fail"));
    EXPECT_TRUE(if_else_action.isReady());

    if_else_action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            if_else_action_run = true;
            loop->exitLoop();
        }
    );
    EXPECT_TRUE(if_else_action.isReady());
    if_else_action.start();

    loop->runLoop();
    EXPECT_TRUE(cond_action_run);
    EXPECT_TRUE(if_else_action_run);
    EXPECT_FALSE(else_action_run);
}

TEST(IfElseAction, CondFailNoElseAction) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    IfElseAction if_else_action(*loop);

    bool cond_action_run = false;
    bool if_action_run = false;
    bool if_else_action_run = false;

    auto cond_action = new FunctionAction(*loop, [&] { cond_action_run = true; return false; });
    auto if_action = new FunctionAction(*loop, [&] { if_action_run = true; return true; });

    EXPECT_TRUE(if_else_action.setChildAs(cond_action, "if"));
    EXPECT_TRUE(if_else_action.setChildAs(if_action, "succ"));
    EXPECT_TRUE(if_else_action.isReady());

    if_else_action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            if_else_action_run = true;
            loop->exitLoop();
        }
    );
    EXPECT_TRUE(if_else_action.isReady());
    if_else_action.start();

    loop->runLoop();
    EXPECT_TRUE(cond_action_run);
    EXPECT_TRUE(if_else_action_run);
    EXPECT_FALSE(if_action_run);
}

TEST(IfElseAction, BlockOnIf) {
    //! 该子动作，第一次启动的时候，会block，后面恢复的时候会finish
    class CanBlockAction : public Action {
      public:
        explicit CanBlockAction(event::Loop &loop) : Action(loop, "CanBlock") { }

        virtual bool isReady() const { return true; }
        virtual void onStart() override { block(1); }
        virtual void onResume() override { finish(true); }
    };

    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    IfElseAction if_else_action(*loop);

    bool is_blocked = false;
    bool succ_action_run = false;
    bool if_else_action_run = false;

    auto cond_action = new CanBlockAction(*loop);
    auto succ_action = new FunctionAction(*loop, [&] {
        succ_action_run = true;
        return true;
    });

    EXPECT_TRUE(if_else_action.setChildAs(cond_action, "if"));
    EXPECT_TRUE(if_else_action.setChildAs(succ_action, "succ"));
    EXPECT_TRUE(if_else_action.isReady());

    if_else_action.setBlockCallback([&] (const Action::Reason &why, const Action::Trace &) {
        is_blocked = true;
        EXPECT_EQ(why.code, 1);
        EXPECT_EQ(if_else_action.state(), Action::State::kPause);
        if_else_action.resume();
    });

    if_else_action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            if_else_action_run = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(if_else_action.isReady());
    if_else_action.start();

    loop->runLoop();

    EXPECT_TRUE(is_blocked);
    EXPECT_TRUE(if_else_action_run);
    EXPECT_TRUE(succ_action_run);
}

TEST(IfElseAction, IsReady) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    IfElseAction if_else_action(*loop);
    EXPECT_FALSE(if_else_action.isReady());

    if_else_action.setChildAs(new SuccAction(*loop), "if");
    EXPECT_FALSE(if_else_action.isReady());

    if_else_action.setChildAs(new SuccAction(*loop), "succ");
    EXPECT_TRUE(if_else_action.isReady());

    if_else_action.setChildAs(new SuccAction(*loop), "fail");
    EXPECT_TRUE(if_else_action.isReady());
}

}
}
