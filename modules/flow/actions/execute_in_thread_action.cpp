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

#include "execute_in_thread_action.h"

#include <tbox/base/log.h>
#include <tbox/base/lifetime_tag.hpp>
#include <tbox/base/json.hpp>
#include <tbox/util/execute_cmd.h>

namespace tbox {
namespace flow {

ExecuteInThreadAction::ExecuteInThreadAction(event::Loop &loop, eventx::ThreadExecutor &thread_executor)
  : Action(loop, "ExecuteInThread")
  , thread_executor_(thread_executor)
{ }

ExecuteInThreadAction::ExecuteInThreadAction(event::Loop &loop, eventx::ThreadExecutor &thread_executor, Func &&func)
  : Action(loop, "ExecuteInThread")
  , thread_executor_(thread_executor)
  , func_(std::move(func))
{ }

void ExecuteInThreadAction::onStart() {
    Action::onStart();

    if (func_) {
        struct Tmp {
            Func func;
            bool succ;
            Reason reason;
        };
        auto tmp = std::make_shared<Tmp>();
        tmp->func = func_;

        auto ltw = ltt_.get();
        task_token_ = thread_executor_.execute(
            [tmp] {
                tmp->succ = tmp->func(tmp->reason);
            },
            [this, tmp, ltw] {
                if (ltw) {
                    finish(tmp->succ, tmp->reason);
                } else {
                    LogWarn("ExecuteInThreadAction object lifetime end");
                }
            }
        );

    } else {
        LogWarn("cmd is empty");
        finish(false);
    }
}

void ExecuteInThreadAction::onStop() {
    auto result = thread_executor_.cancel(task_token_);
    if (result != eventx::ThreadExecutor::CancelResult::kExecuting)
        LogWarn("action %d:%s[%s] stop fail", id(), type().c_str(), label().c_str());

    Action::onStop();
}

void ExecuteInThreadAction::onPause() {
    LogWarn("action %d:%s[%s] can't pause", id(), type().c_str(), label().c_str());
    Action::onPause();
}

void ExecuteInThreadAction::onResume() {
    Action::onResume();
    LogWarn("action %d:%s[%s] can't resume", id(), type().c_str(), label().c_str());
}

void ExecuteInThreadAction::onReset() {
    task_token_.reset();
    Action::onReset();
}

void ExecuteInThreadAction::toJson(Json &js) const {
    Action::toJson(js);

    js["task_token"] = task_token_.id();
}

}
}
