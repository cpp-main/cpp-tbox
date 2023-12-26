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
    WrapperAction action(*loop);

    bool is_callback = false;
    EXPECT_TRUE(action.setChild(new SuccAction(*loop)));
    EXPECT_TRUE(action.isReady());
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_TRUE(succ);
            is_callback = true;
        }
    );

    action.start();

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, NormalFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    WrapperAction action(*loop);

    bool is_callback = false;
    action.setChild(new FailAction(*loop));
    EXPECT_TRUE(action.isReady());
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_FALSE(succ);
            is_callback = true;
        }
    );

    action.start();

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}


TEST(WrapperAction, InvertSucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    WrapperAction action(*loop, WrapperAction::Mode::kInvert);

    bool is_callback = false;
    action.setChild(new SuccAction(*loop));
    EXPECT_TRUE(action.isReady());
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_FALSE(succ);
            is_callback = true;
        }
    );

    action.start();

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, InvertFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    WrapperAction action(*loop, WrapperAction::Mode::kInvert);

    bool is_callback = false;
    action.setChild(new FailAction(*loop));
    EXPECT_TRUE(action.isReady());
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_TRUE(succ);
            is_callback = true;
        }
    );

    action.start();

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, AlwaySuccSucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    WrapperAction action(*loop, WrapperAction::Mode::kAlwaySucc);

    bool is_callback = false;
    action.setChild(new SuccAction(*loop));
    EXPECT_TRUE(action.isReady());
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_TRUE(succ);
            is_callback = true;
        }
    );

    action.start();

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, AlwaySuccFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    WrapperAction action(*loop, WrapperAction::Mode::kAlwaySucc);

    bool is_callback = false;
    action.setChild(new FailAction(*loop));
    EXPECT_TRUE(action.isReady());
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_TRUE(succ);
            is_callback = true;
        }
    );

    action.start();

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, AlwayFailSucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    WrapperAction action(*loop, WrapperAction::Mode::kAlwayFail);

    bool is_callback = false;
    action.setChild(new SuccAction(*loop));
    EXPECT_TRUE(action.isReady());
    action.setFinishCallback(
        [&](bool succ) {
            EXPECT_FALSE(succ);
            is_callback = true;
        }
    );

    action.start();

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, AlwayFailFail) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    WrapperAction action(*loop, WrapperAction::Mode::kAlwayFail);

    bool is_callback = false;
    action.setChild(new FailAction(*loop));
    EXPECT_TRUE(action.isReady());
    action.setFinishCallback(
        [&](bool succ) {
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
