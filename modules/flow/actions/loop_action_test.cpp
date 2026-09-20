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
#include <iostream>
#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/json.hpp>

#include "loop_action.h"
#include "function_action.h"
#include "sleep_action.h"
#include "sequence_action.h"
#include "succ_fail_action.h"

namespace tbox {
namespace flow {

TEST(LoopAction, IsReady) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    LoopAction action(*loop);
    EXPECT_FALSE(action.isReady());

    action.setChild(new SuccAction(*loop));
    EXPECT_TRUE(action.isReady());
}

/**
 *  int loop_times = 0;
 *  while (true) {
 *    ++loop_times;
 *    return true;
 *  }
 */
TEST(LoopAction, FunctionActionForever) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    LoopAction loop_action(*loop, LoopAction::Mode::kForever);

    int loop_times = 0;
    auto function_action = new FunctionAction(*loop,
        [&] {
            ++loop_times;
            return true;
        }
    );
    bool is_finished = false;

    EXPECT_TRUE(loop_action.setChild(function_action));
    EXPECT_TRUE(loop_action.isReady());
    loop_action.setFinishCallback([&] (bool, const Action::Reason&, const Action::Trace&) { is_finished = true; });

    loop_action.start();
    loop->exitLoop(std::chrono::milliseconds(1000));
    loop->runLoop();
    loop_action.stop();

    EXPECT_FALSE(is_finished);
    EXPECT_GT(loop_times, 1000);
}

/**
 *  int loop_times = 0;
 *  while (true) {
 *    ++loop_times;
 *    Sleep(100);
 *  };
 */
TEST(LoopAction, SleepActionForever) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    LoopAction loop_action(*loop, LoopAction::Mode::kForever);

    int loop_times = 0;
    auto function_action = new FunctionAction(*loop,
        [&] {
            ++loop_times;
            return true;
        }
    );
    auto delay_10ms_action = new SleepAction(*loop, std::chrono::milliseconds(100));
    auto seq_action = new SequenceAction(*loop);
    seq_action->addChild(delay_10ms_action);
    seq_action->addChild(function_action);

    bool is_finished = false;

    EXPECT_TRUE(loop_action.setChild(seq_action));
    EXPECT_TRUE(loop_action.isReady());
    loop_action.setFinishCallback([&] (bool, const Action::Reason&, const Action::Trace&) { is_finished = true; });

    loop_action.start();
    loop->exitLoop(std::chrono::milliseconds(1010));
    loop->runLoop();
    loop_action.stop();

    EXPECT_FALSE(is_finished);
    EXPECT_EQ(loop_times, 10);
    loop->cleanup();
}

TEST(LoopAction, ProcessForwarding) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    class TestAction : public Action {
      public:
        explicit TestAction(event::Loop &loop) : Action(loop, "Test") { }
        virtual bool isReady() const override { return true; }
        virtual void onStart() override {
            Action::onStart();

            // 上报进度
            nlohmann::json progress;
            progress["iteration"] = 1;
            progress["status"] = "running";
            process(progress);

            // 完成动作
            finish(true);
        }
    };

    LoopAction loop_action(*loop, LoopAction::Mode::kForever);
    auto child_action = new TestAction(*loop);
    EXPECT_TRUE(loop_action.setChild(child_action));

    bool is_process_callback = false;
    bool is_finish_callback = false;

    loop_action.setProcessCallback(
        [&](const Json &js_process, const Action::Trace &t) {
            EXPECT_EQ(js_process.at("iteration"), 1);
            EXPECT_EQ(js_process.at("status"), "running");
            ASSERT_EQ(t.size(), 2);  // TestAction -> LoopAction
            EXPECT_EQ(t[0].type, "Test");
            EXPECT_EQ(t[1].type, "Loop");
            is_process_callback = true;
        }
    );

    loop_action.setFinishCallback(
        [&](bool is_succ, const Action::Reason &, const Action::Trace &) {
            // 我们不会到达这里，因为我们会在测试完成后停止
            is_finish_callback = true;
        }
    );

    EXPECT_TRUE(loop_action.isReady());
    loop_action.start();

    loop->exitLoop(std::chrono::milliseconds(10));
    loop->runLoop();
    loop_action.stop();

    EXPECT_TRUE(is_process_callback);
    // 不检查is_finish_callback，因为我们主动停止了循环
}

}
}
