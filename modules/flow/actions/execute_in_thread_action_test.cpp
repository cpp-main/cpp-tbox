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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
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
#include <tbox/eventx/work_thread.h>

#include "execute_in_thread_action.h"

namespace tbox {
namespace flow {

TEST(ExecuteInThreadAction, IsReady) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    eventx::WorkThread worker(loop);

    ExecuteInThreadAction action(*loop, worker);
    EXPECT_FALSE(action.isReady());

    action.setFunc([] (Action::Reason &) { return true; });
    EXPECT_TRUE(action.isReady());
}

TEST(ExecuteInThreadAction, Succ) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    eventx::WorkThread worker(loop);

    ExecuteInThreadAction action(*loop, worker);
    action.setFunc(
        [] (Action::Reason &) {
            return true;
        }
    );

    bool is_callback = false;
    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            EXPECT_EQ(r.code, 0);

            is_callback = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();
    EXPECT_TRUE(is_callback);
}

TEST(ExecuteInThreadAction, Fail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    eventx::WorkThread worker(loop);

    ExecuteInThreadAction action(*loop, worker);
    action.setFunc(
        [] (Action::Reason &r) {
            r.code = 101;
            return false;
        }
    );

    bool is_callback = false;
    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &) {
            EXPECT_FALSE(is_succ);
            EXPECT_EQ(r.code, 101);

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
