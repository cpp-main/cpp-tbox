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

#include "assemble_action.h"
#include <tbox/base/log.h>

namespace tbox {
namespace flow {

int AssembleAction::addChild(Action *)
{
  LogWarn("%d:%s[%s] not implement this function", id(), type().c_str(), label().c_str());
  return -1;
}

bool AssembleAction::setChild(Action *)
{
  LogWarn("%d:%s[%s] not implement this function", id(), type().c_str(), label().c_str());
  return false;
}

bool AssembleAction::setChildAs(Action *, const std::string &)
{
  LogWarn("%d:%s[%s] not implement this function", id(), type().c_str(), label().c_str());
  return false;
}

void AssembleAction::onFinal()
{
    if (final_cb_)
        final_cb_();
}

}
}
