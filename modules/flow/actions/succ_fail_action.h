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
#ifndef TBOX_FLOW_SUCC_FAIL_ACTION_H_20230304
#define TBOX_FLOW_SUCC_FAIL_ACTION_H_20230304

#include "../action.h"

namespace tbox {
namespace flow {

/// 成功动作
class SuccAction : public Action {
  public:
    SuccAction(event::Loop &loop) : Action(loop, "Succ") { }

  protected:
    virtual bool onStart() {
      finish(true);
      return true;
    }
};

/// 失败动作
class FailAction : public Action {
  public:
    FailAction(event::Loop &loop) : Action(loop, "Fail") { }

  protected:
    virtual bool onStart() {
        finish(false);
        return true;
    }
};

}
}

#endif //TBOX_FLOW_SUCC_FAIL_ACTION_H_20230304
