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
#include <tbox/eventx/timer_pool.h>
#include <tbox/base/scope_exit.hpp>

#include "if_then_action.h"
#include "function_action.h"
#include "succ_fail_action.h"
#include "dummy_action.h"

namespace tbox {
namespace flow {

TEST(IfThenAction, AddChild) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    IfThenAction action(*loop);

    auto if_action = new SuccAction(*loop);
    EXPECT_EQ(action.addChildAs(if_action, "xxx"), -1);  //! 填错了不行
    EXPECT_EQ(action.addChildAs(if_action, "then"), -1); //! 必须先add if
    EXPECT_EQ(action.addChildAs(if_action, "if"), 0);    //! 添加 if 能成功
    EXPECT_EQ(action.addChildAs(if_action, "then"), -1); //! 不能重复add
    EXPECT_EQ(action.addChildAs(new SuccAction(*loop), "then"), 0);
}

TEST(IfThenAction, IsReady) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    IfThenAction action(*loop);
    EXPECT_FALSE(action.isReady());

    EXPECT_EQ(action.addChildAs(new FailAction(*loop), "if"), 0);
    EXPECT_FALSE(action.isReady());

    EXPECT_EQ(action.addChildAs(new SuccAction(*loop), "then"), 0);
    EXPECT_TRUE(action.isReady());

    EXPECT_EQ(action.addChildAs(new FailAction(*loop), "if"), 1);
    EXPECT_FALSE(action.isReady()); //! 在没有完成一对if-then之前，不会处于ready状态

    EXPECT_EQ(action.addChildAs(new SuccAction(*loop), "then"), 1);
    EXPECT_TRUE(action.isReady());
}

/**
 * 实现以下逻辑
 * if (if_action(true)) {
 *   return then_action(true);
 * }
 * return false;
 */
TEST(IfThenAction, IfTrueThen) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    IfThenAction action(*loop);

    bool if_action_run = false;
    bool then_action_run = false;
    bool all_finish_run = false;

    auto if_action = new FunctionAction(*loop, [&] { if_action_run = true; return true; });
    auto then_action = new FunctionAction(*loop,
        [&] (Action::Reason &r) {
            r.code = 101;
            r.message = "test";
            then_action_run = true;
            return true;
        }
    );

    action.addChildAs(if_action, "if");
    action.addChildAs(then_action, "then");

    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            EXPECT_EQ(r.code, 101);
            EXPECT_EQ(r.message, "test");
            all_finish_run = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();

    EXPECT_TRUE(if_action_run);
    EXPECT_TRUE(then_action_run);
    EXPECT_TRUE(all_finish_run);
}

/**
 * 实现以下逻辑
 * if (if_action(false)) {
 *   reutrn then_action(true, 101, "then_action");
 * } else {
 *   reutrn else_action(true, 102, "else_action");
 * }
 * return false;
 */
TEST(IfThenAction, IfFalseElse) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });

    IfThenAction action(*loop);

    bool if_action_run = false;
    bool then_action_run = false;
    bool else_action_run = false;
    bool all_finish_run = false;

    auto if_action = new FunctionAction(*loop, [&] { if_action_run = true; return false; });
    auto then_action = new FunctionAction(*loop,
        [&] (Action::Reason &r) {
            r.code = 101;
            r.message = "then_action";
            then_action_run = true;
            return true;
        }
    );

    auto else_action = new FunctionAction(*loop,
        [&] (Action::Reason &r) {
            r.code = 102;
            r.message = "else_action";
            else_action_run = true;
            return true;
        }
    );

    action.addChildAs(if_action, "if");
    action.addChildAs(then_action, "then");
    action.addChildAs(new SuccAction(*loop), "if");
    action.addChildAs(else_action, "then");

    action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &r, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            EXPECT_EQ(r.code, 102);
            EXPECT_EQ(r.message, "else_action");
            all_finish_run = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(action.isReady());
    action.start();

    loop->runLoop();

    EXPECT_TRUE(if_action_run);
    EXPECT_FALSE(then_action_run);
    EXPECT_TRUE(else_action_run);
    EXPECT_TRUE(all_finish_run);
}

/**
* 实现以下逻辑
* if (if_action(false)) {
*   reutrn then_action(true, 101, "then_action");
* }
* return false;
*/
TEST(IfThenAction, IfFalse) {
   auto loop = event::Loop::New();
   SetScopeExitAction([loop] { delete loop; });

   IfThenAction action(*loop);

   bool if_action_run = false;
   bool then_action_run = false;
   bool all_finish_run = false;

   auto if_action = new FunctionAction(*loop, [&] { if_action_run = true; return false; });
   auto then_action = new FunctionAction(*loop,
       [&] (Action::Reason &r) {
           r.code = 101;
           r.message = "then_action";
           then_action_run = true;
           return true;
       }
   );

   action.addChildAs(if_action, "if");
   action.addChildAs(then_action, "then");

   action.setFinishCallback(
       [&] (bool is_succ, const Action::Reason &r, const Action::Trace &) {
           EXPECT_FALSE(is_succ);
           EXPECT_EQ(r.code, ACTION_REASON_IF_THEN_SKIP);
           EXPECT_EQ(r.message, "IfThenSkip");
           all_finish_run = true;
           loop->exitLoop();
       }
   );

   EXPECT_TRUE(action.isReady());
   action.start();

   loop->runLoop();

   EXPECT_TRUE(if_action_run);
   EXPECT_FALSE(then_action_run);
   EXPECT_TRUE(all_finish_run);
}

TEST(IfThenAction, FinishPauseOnIf) {
    auto loop = event::Loop::New();
    SetScopeExitAction([loop] { delete loop; });
    eventx::TimerPool timer_pool(loop);

    IfThenAction if_then_action(*loop);

    bool then_action_run = false;
    bool if_then_action_run = false;
    bool do_resume = false;

    auto if_action = new DummyAction(*loop);
    auto then_action = new FunctionAction(*loop, [&] {
        then_action_run = true;
        return true;
    });

    EXPECT_EQ(if_then_action.addChildAs(if_action, "if"), 0);
    EXPECT_EQ(if_then_action.addChildAs(then_action, "then"), 0);
    EXPECT_TRUE(if_then_action.isReady());

    if_action->setStartCallback([&] {
        timer_pool.doAfter(std::chrono::milliseconds(1), [&] {
            //! 同时发生动作结束与动作暂停的事件
            if_action->emitFinish(true);
            if_then_action.pause();
        });
        timer_pool.doAfter(std::chrono::milliseconds(10), [&] {
            do_resume = true;
            if_then_action.resume();
        });
    });

    if_then_action.setFinishCallback(
        [&] (bool is_succ, const Action::Reason &, const Action::Trace &) {
            EXPECT_TRUE(is_succ);
            if_then_action_run = true;
            loop->exitLoop();
        }
    );

    EXPECT_TRUE(if_then_action.isReady());
    if_then_action.start();

    loop->runLoop();

    EXPECT_TRUE(if_then_action_run);
    EXPECT_TRUE(then_action_run);
    EXPECT_TRUE(do_resume);
}

}
}
