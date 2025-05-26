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

#include "execute_cmd_action.h"

#include <tbox/base/log.h>
#include <tbox/base/lifetime_tag.hpp>
#include <tbox/base/json.hpp>
#include <tbox/util/execute_cmd.h>

namespace tbox {
namespace flow {

ExecuteCmdAction::ExecuteCmdAction(event::Loop &loop, eventx::ThreadExecutor &thread_executor)
  : Action(loop, "ExecuteCmd")
  , thread_executor_(thread_executor)
{ }

ExecuteCmdAction::ExecuteCmdAction(event::Loop &loop, eventx::ThreadExecutor &thread_executor, const std::string &cmd)
  : Action(loop, "ExecuteCmd")
  , thread_executor_(thread_executor)
  , cmd_(cmd)
{ }

void ExecuteCmdAction::onStart() {
    Action::onStart();

    if (!cmd_.empty()) {
        struct Tmp {
            std::string cmd;
            bool succ;
            int return_code;
            std::string std_output;
        };
        auto tmp = std::make_shared<Tmp>();
        tmp->cmd = cmd_;

        auto ltw = ltt_.get();
        task_token_ = thread_executor_.execute(
            [tmp] {
                tmp->succ = util::ExecuteCmd(tmp->cmd, tmp->std_output, tmp->return_code);
            },
            [this, tmp, ltw] {
                if (ltw) {
                    std_output_ = tmp->std_output;
                    reture_code_ = tmp->return_code;
                    finish(tmp->succ);
                } else {
                    LogWarn("ExecuteCmdAction object lifetime end");
                }
            }
        );

    } else {
        LogWarn("cmd is empty");
        finish(false);
    }
}

void ExecuteCmdAction::onStop() {
    auto result = thread_executor_.cancel(task_token_);
    if (result != eventx::ThreadExecutor::CancelResult::kExecuting)
        LogWarn("stop fail, cmd: '%s'", cmd_.c_str());

    Action::onStop();
}

void ExecuteCmdAction::onPause() {
    LogWarn("can't pause");
    Action::onPause();
}

void ExecuteCmdAction::onResume() {
    Action::onResume();
    LogWarn("can't pause");
}

void ExecuteCmdAction::onReset() {
    task_token_.reset();
    reture_code_ = 0;
    std_output_.clear();

    Action::onReset();
}

void ExecuteCmdAction::toJson(Json &js) const {
    Action::toJson(js);

    js["cmd"] = cmd_;
    js["task_token"] = task_token_.id();
    js["reture_code"] = reture_code_;
    js["std_output"] = std_output_;
}

}
}
