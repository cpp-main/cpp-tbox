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
#include <tbox/base/json.hpp>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>

#include "function_action.h"
#include "sequence_action.h"

namespace tbox {
namespace flow {

TEST(FunctionAction, True) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    FunctionAction action(*loop, [] { return true; });
    bool is_callback = false;
    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_TRUE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FUNCTION_ACTION);
            EXPECT_EQ(r.message, "FunctionAction");
            ASSERT_EQ(t.size(), 1);
            EXPECT_EQ(t[0].id, action.id());

            is_callback = true;
            loop->exitLoop();
        }
    );
    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();
    EXPECT_TRUE(is_callback);
}

TEST(FunctionAction, False) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    FunctionAction action(*loop, [] { return false; });
    bool is_callback = false;
    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_FALSE(is_succ);
            EXPECT_EQ(r.code, ACTION_REASON_FUNCTION_ACTION);
            EXPECT_EQ(r.message, "FunctionAction");
            ASSERT_EQ(t.size(), 1);
            EXPECT_EQ(t[0].id, action.id());

            is_callback = true;
            loop->exitLoop();
        }
    );
    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();
    EXPECT_TRUE(is_callback);
}

TEST(FunctionAction, WithReason) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    FunctionAction action(*loop,
      [] (Action::Reason &r) {
        r.code = 1001;
        r.message = "test";
        return false;
      }
    );

    bool is_callback = false;
    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &) {
            EXPECT_FALSE(is_succ);
            EXPECT_EQ(r.code, 1001);
            EXPECT_EQ(r.message, "test");
            is_callback = true;
            loop->exitLoop();
        }
    );
    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();
    EXPECT_TRUE(is_callback);
}

TEST(FunctionAction, WithVars) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    SequenceAction seq(*loop, SequenceAction::Mode::kAnyFail);

    seq.vars().define("r1", false);
    seq.vars().define("r2", 0);

    auto func1 = new FunctionAction(*loop,
      [] (util::Variables &v) {
        v.set("r1", true);
        return true;
      }
    );
    auto func2 = new FunctionAction(*loop,
      [] (Action::Reason &r, util::Variables &v) {
        r.code = 1001;
        r.message = "test";
        v.set("r2", 200);
        return true;
      }
    );

    seq.addChild(func1);
    seq.addChild(func2);

    bool is_callback = false;
    seq.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            is_callback = true;
            EXPECT_EQ(r.code, 1001);
            EXPECT_EQ(r.message, "test");
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(seq.isReady());
    seq.start();

    loop->runLoop();
    EXPECT_TRUE(is_callback);

    {
        Json js;
        EXPECT_TRUE(seq.vars().get("r1", js));
        EXPECT_TRUE(js.is_boolean());
        EXPECT_TRUE(js.get<bool>());
    }
    {
        Json js;
        EXPECT_TRUE(seq.vars().get("r2", js));
        EXPECT_TRUE(js.is_number());
        EXPECT_EQ(js.get<int>(), 200);
    }
}

}
}
