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

#include "function_action.h"

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

}
}
