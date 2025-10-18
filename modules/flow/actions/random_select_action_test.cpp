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

#include "random_select_action.h"
#include "function_action.h"
#include "succ_fail_action.h"
#include "repeat_action.h"

namespace tbox {
namespace flow {

TEST(RandomSelectAction, IsReady) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    RandomSelectAction random_select_action(*loop);
    EXPECT_FALSE(random_select_action.isReady());

    random_select_action.addChild(new SuccAction(*loop));
    EXPECT_TRUE(random_select_action.isReady());

    random_select_action.addChild(new FailAction(*loop));
    EXPECT_TRUE(random_select_action.isReady());
}

TEST(RandomSelectAction, OneChild) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    RandomSelectAction random_select_action(*loop);
    random_select_action.addChild(new SuccAction(*loop));

    loop->exitLoop(std::chrono::milliseconds(10));

    bool is_finished = false;
    random_select_action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &, const Action::Trace &) {
            is_finished = true;
            EXPECT_TRUE(is_succ);
        }
    );
    random_select_action.start();

    loop->runLoop();
    EXPECT_TRUE(is_finished);
}

//! 在 RandomSelectAction 下添加 10 个 FunctionAction。
//! 用 RepeatAction 反复执行 100 次，观察是否每个 FunctionAction 都被执行过
TEST(RandomSelectAction, TenChildren) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    std::set<int> run_mark;
    RepeatAction repeat_action(*loop, 100);

    auto random_select_action = new RandomSelectAction(*loop);
    repeat_action.setChild(random_select_action);

    for (int i = 0; i < 10; ++i) {
        random_select_action->addChild(
            new FunctionAction(*loop, [&, i] { run_mark.insert(i); return true; })
        );
    }
    repeat_action.start();

    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();

    //! 检查是否每个FunctionAction都被执行到
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(run_mark.count(i), 1);
    }
}

}
}
