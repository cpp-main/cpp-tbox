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

FunctionAction::FunctionAction(event::Loop &loop, const Func &func) :
  Action(loop, "Function"), func_(func)
{
  TBOX_ASSERT(func != nullptr);
}

bool FunctionAction::onStart() {
  finish(func_());
  return true;
}

}
}
