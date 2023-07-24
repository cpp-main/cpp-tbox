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

#include "wrapper_action.h"
#include "succ_fail_action.h"

namespace tbox {
namespace flow {

TEST(WrapperAction, NormalSucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    WrapperAction action(*loop, new SuccAction(*loop));
    action.start();

    bool is_callback = false;
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_TRUE(succ);
            is_callback = true;
        }
    );

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, NormalFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    WrapperAction action(*loop, new FailAction(*loop));
    action.start();

    bool is_callback = false;
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_FALSE(succ);
            is_callback = true;
        }
    );

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}


TEST(WrapperAction, InvertSucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    WrapperAction action(*loop, new SuccAction(*loop), WrapperAction::Mode::kInvert);
    action.start();

    bool is_callback = false;
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_FALSE(succ);
            is_callback = true;
        }
    );

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, InvertFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    WrapperAction action(*loop, new FailAction(*loop), WrapperAction::Mode::kInvert);
    action.start();

    bool is_callback = false;
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_TRUE(succ);
            is_callback = true;
        }
    );

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, AlwaySuccSucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    WrapperAction action(*loop, new SuccAction(*loop), WrapperAction::Mode::kAlwaySucc);
    action.start();

    bool is_callback = false;
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_TRUE(succ);
            is_callback = true;
        }
    );

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, AlwaySuccFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    WrapperAction action(*loop, new FailAction(*loop), WrapperAction::Mode::kAlwaySucc);
    action.start();

    bool is_callback = false;
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_TRUE(succ);
            is_callback = true;
        }
    );

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, AlwayFailSucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    WrapperAction action(*loop, new SuccAction(*loop), WrapperAction::Mode::kAlwayFail);
    action.start();

    bool is_callback = false;
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_FALSE(succ);
            is_callback = true;
        }
    );

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, AlwayFailFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    WrapperAction action(*loop, new FailAction(*loop), WrapperAction::Mode::kAlwayFail);
    action.start();

    bool is_callback = false;
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_FALSE(succ);
            is_callback = true;
        }
    );

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

}
}
