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
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
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

#include "assemble_action.h"

namespace tbox {
namespace flow {

namespace {

class TestAction : public AssembleAction {
  public:
    explicit TestAction(tbox::event::Loop &loop)
      : AssembleAction(loop, "test") { }

    virtual bool isReady() const { return true; }

    void finishMe() { finish(true); }
};

}

//! 测试在被stop()的时候会不会调FinalCallback
TEST(AssembleAction, FinalCallbackOnStop) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    TestAction action(*loop);

    bool is_invoked = false;
    action.setFinalCallback([&] { is_invoked = true; });
    action.start();
    action.stop();

    EXPECT_TRUE(is_invoked);
}

//! 测试在被finish()的时候会不会调FinalCallback
TEST(AssembleAction, FinalCallbackOnStopOnFinish) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    TestAction action(*loop);

    bool is_invoked = false;
    action.setFinalCallback([&] { is_invoked = true; });
    action.start();

    loop->run([&] { action.finishMe(); });
    loop->exitLoop(std::chrono::milliseconds(10));

    loop->runLoop();

    EXPECT_TRUE(is_invoked);
}

}
}
