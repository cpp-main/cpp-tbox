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
#include <tbox/base/json.hpp>

#include "wrapper_action.h"
#include "succ_fail_action.h"

namespace tbox {
namespace flow {

TEST(WrapperAction, IsReady) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    WrapperAction action(*loop);
    EXPECT_FALSE(action.isReady());

    action.setChild(new SuccAction(*loop));
    EXPECT_TRUE(action.isReady());
}

TEST(WrapperAction, NormalSucc) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    WrapperAction action(*loop);

    bool is_callback = false;
    EXPECT_TRUE(action.setChild(new SuccAction(*loop)));
    EXPECT_TRUE(action.isReady());
    action.setFinishCallback(
        [&](bool succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_TRUE(succ);
            EXPECT_EQ(r.code, ACTION_REASON_SUCC_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Succ");
            EXPECT_EQ(t[1].type, "Wrapper");
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
        [&](bool succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_FALSE(succ);
            EXPECT_EQ(r.code, ACTION_REASON_FAIL_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Fail");
            EXPECT_EQ(t[1].type, "Wrapper");
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
        [&](bool succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_FALSE(succ);
            EXPECT_EQ(r.code, ACTION_REASON_SUCC_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Succ");
            EXPECT_EQ(t[1].type, "Wrapper");
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
        [&](bool succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_TRUE(succ);
            EXPECT_EQ(r.code, ACTION_REASON_FAIL_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Fail");
            EXPECT_EQ(t[1].type, "Wrapper");
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
        [&](bool succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_TRUE(succ);
            EXPECT_EQ(r.code, ACTION_REASON_SUCC_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Succ");
            EXPECT_EQ(t[1].type, "Wrapper");
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
        [&](bool succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_TRUE(succ);
            EXPECT_EQ(r.code, ACTION_REASON_FAIL_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Fail");
            EXPECT_EQ(t[1].type, "Wrapper");
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
        [&](bool succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_FALSE(succ);
            EXPECT_EQ(r.code, ACTION_REASON_SUCC_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Succ");
            EXPECT_EQ(t[1].type, "Wrapper");
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
        [&](bool succ, const Action::Reason &r, const Action::Trace &t) {
            EXPECT_FALSE(succ);
            EXPECT_EQ(r.code, ACTION_REASON_FAIL_ACTION);
            EXPECT_EQ(t.size(), 2u);
            EXPECT_EQ(t[0].type, "Fail");
            EXPECT_EQ(t[1].type, "Wrapper");
            is_callback = true;
        }
    );

    action.start();

    loop->exitLoop(std::chrono::milliseconds(1));
    loop->runLoop();

    EXPECT_TRUE(is_callback);
}

TEST(WrapperAction, ProcessForwarding) {
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
            progress["transformation"] = "applying";
            progress["result"] = "wrapped";
            process(progress);

            // 完成动作
            finish(true);
        }
    };

    WrapperAction wrapper_action(*loop, WrapperAction::Mode::kNormal);

    bool is_process_callback = false;
    bool is_finish_callback = false;

    auto child_action = new TestAction(*loop);
    wrapper_action.setChild(child_action);

    wrapper_action.setProcessCallback(
        [&](const Json &js_process, const Action::Trace &t) {
            EXPECT_EQ(js_process.at("transformation"), "applying");
            EXPECT_EQ(js_process.at("result"), "wrapped");

            // Trace应该包含TestAction -> WrapperAction
            ASSERT_GE(t.size(), 2);
            EXPECT_EQ(t[t.size()-2].type, "Test");
            EXPECT_EQ(t[t.size()-1].type, "Wrapper");
            is_process_callback = true;
        }
    );

    wrapper_action.setFinishCallback(
        [&](bool is_succ, const Action::Reason &, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            is_finish_callback = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(wrapper_action.isReady());
    wrapper_action.start();

    loop->runLoop();

    EXPECT_TRUE(is_process_callback);
    EXPECT_TRUE(is_finish_callback);
}

}
}
