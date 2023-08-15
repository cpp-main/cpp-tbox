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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_COROUTINE_BROADCAST_HPP_20180527
#define TBOX_COROUTINE_BROADCAST_HPP_20180527

#include <vector>
#include "scheduler.h"

namespace tbox {
namespace coroutine {

//! 广播信号
class Broadcast {
  public:
    Broadcast(Scheduler &sch) : sch_(sch) { }

    bool wait() {
        wait_tokens_.push_back(sch_.getToken());
        sch_.wait();
        return !sch_.isCanceled();
    }

    void post() {
        for (auto t : wait_tokens_)
            sch_.resume(t);
        wait_tokens_.clear();
    }

  private:
    Scheduler &sch_;
    std::vector<RoutineToken> wait_tokens_;
};

}
}

#endif //TBOX_COROUTINE_BROADCAST_HPP_20180527
