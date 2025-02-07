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
#include "function_action.h"
#include <tbox/base/assert.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace flow {

FunctionAction::FunctionAction(event::Loop &loop)
  : Action(loop, "Function")
{ }

FunctionAction::FunctionAction(event::Loop &loop, Func &&func)
  : Action(loop, "Function")
  , func_(std::move(func))
{
    TBOX_ASSERT(func_ != nullptr);
}

FunctionAction::FunctionAction(event::Loop &loop, FuncWithReason &&func)
  : Action(loop, "Function")
  , func_with_reason_(std::move(func))
{
    TBOX_ASSERT(func_with_reason_ != nullptr);
}

FunctionAction::FunctionAction(event::Loop &loop, FuncWithVars &&func)
  : Action(loop, "Function")
  , func_with_vars_(std::move(func))
{
    TBOX_ASSERT(func_with_vars_ != nullptr);
}

FunctionAction::FunctionAction(event::Loop &loop, FuncWithReasonVars &&func)
  : Action(loop, "Function")
  , func_with_reason_vars_(std::move(func))
{
    TBOX_ASSERT(func_with_reason_vars_ != nullptr);
}

void FunctionAction::onStart() {
    Action::onStart();

    Reason reason(ACTION_REASON_FUNCTION_ACTION, "FunctionAction");

    if (func_) {
      finish(func_(), reason);

    } else if (func_with_reason_) {
      finish(func_with_reason_(reason), reason);

    } else if (func_with_vars_) {
      finish(func_with_vars_(vars()), reason);

    } else if (func_with_reason_vars_) {
      finish(func_with_reason_vars_(reason, vars()), reason);

    } else {
      TBOX_ASSERT(isReady());
    }
}

}
}
