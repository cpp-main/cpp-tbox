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
#ifndef TBOX_FLOW_DUMMY_ACTION_H_20241107
#define TBOX_FLOW_DUMMY_ACTION_H_20241107

#include "../action.h"

namespace tbox {
namespace flow {

//! 木偶动作
//! 其自身不会主动发起结束动作，也不会触发阻塞。由外部通过调用emitFinish()与emitBlock()来实现
class DummyAction : public Action {
  public:
    explicit DummyAction(event::Loop &loop) : Action(loop, "Dummy") {}

    virtual bool isReady() const { return true; }

    inline void emitFinish(bool is_succ, const Reason &reason = Reason()) { finish(is_succ, reason); }
    inline void emitBlock(const Reason &reason) { block(reason); }
};

}
}

#endif //TBOX_FLOW_DUMMY_ACTION_H_20241107
