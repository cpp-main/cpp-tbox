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
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

#include "succ_fail_action.h"

namespace tbox {
namespace flow {

TEST(SuccAction, base) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    SuccAction action(*loop);

    bool is_callback = false;
    action.setFinishCallback(
        [&](bool succ, const Action::Reason&, const Action::Trace&) {
            EXPECT_TRUE(succ);
            is_callback = true;
        }
    );

    action.start();
    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(FailAction, base) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    FailAction action(*loop);

    bool is_callback = false;
    action.setFinishCallback(
        [&](bool succ, const Action::Reason&, const Action::Trace&) {
            EXPECT_FALSE(succ);
            is_callback = true;
        }
    );

    action.start();
    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

}
}
